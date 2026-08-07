/**
  ******************************************************************************
  * @file    Optical_flow.c
  * @brief   ATK-PMW3901 光流模块驱动实现
  *          - PMW3901 光流: SPI1+DMA (PA5/6/7, PA4=CS, Mode3, 1.5625MHz)
  *          - VL53LXX 激光: I2C1 轮询 (PB6/PB7, 400kHz, 默认模式 + 30ms预算)
  *          基于 ATK-PMW3901 用户手册 V1.1 + PMW3901MB Datasheet R1.10
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Optical_flow.h"
#include "spi.h"              /* hspi1, CubeMX 生成 */
#include "i2c.h"              /* hi2c1, CubeMX 生成 */
#include "Location_deal.h"    /* pitch, roll 全局变量 (extern) */
#include "SEGGER_RTT.h"       /* SEGGER_RTT_printf, 调试输出 */
#include <stdbool.h>           /* bool, true, false (ARMCC5 C90 模式需要) */
#include <string.h>
#include <math.h>             /* tanf, fabsf */

/*============================================================================
 * 宏定义 — 引脚
 *============================================================================*/
#define CS_PORT              GPIOA
#define CS_PIN               GPIO_PIN_4

/*============================================================================
 * 宏定义 — PMW3901 寄存器地址
 *============================================================================*/
#define REG_PRODUCT_ID        0x00    /* 产品ID, 应读回 0x49 */
#define REG_MOTION            0x02    /* 运动标志位 */
#define REG_DELTA_X_L         0x03    /* DeltaX 低字节 */
#define REG_DELTA_Y_L         0x05    /* DeltaY 低字节 */
#define REG_SQUAL             0x07    /* 表面质量 */
#define REG_RAW_DATA_SUM      0x08    /* 原始数据求和 */
#define REG_MAX_RAW_DATA      0x09    /* 最大原始数据 */
#define REG_MIN_RAW_DATA      0x0A    /* 最小原始数据 */
#define REG_SHUTTER_UPPER     0x0B    /* 快门高字节 */
#define REG_MOTION_BURST      0x16    /* 12字节突发读起点 */
#define REG_POWER_UP_RESET    0x3A    /* 写 0x5A 触发上电复位 */
#define REG_INV_PRODUCT_ID    0x5F    /* 反码产品ID, 应读回 0xB6 */
#define REG_BANK_SELECT       0x7F    /* 寄存器组(Bank)切换 */

/*============================================================================
 * 宏定义 — 数据换算与滤波参数 (来源: 手册 opticalFlowTask + getOpFlowData)
 *============================================================================*/
#define RESOLUTION            0.2131946f  /* 1m高度下1像素对应位移(cm) */
#define TILT_COMP_COEF        480.0f      /* 倾角补偿系数 (需实测标定) */
#define VEL_LPF_COEF          0.15f       /* 速度低通系数 */
#define VEL_LIMIT             80.0f       /* 速度限幅 (cm/s) */
#define OUTLIER_LIMIT         200         /* 单帧像素异常值限幅 */
#define FAULT_THRESHOLD       100         /* 连续故障帧数 (100帧=1秒) */
#define HEIGHT_MIN            0.05f       /* 最小有效高度 (m), 5cm */
#define HEIGHT_MAX            4.0f        /* 最大有效高度 (m), 4m */
#define DEFAULT_HEIGHT_M      0.15f       /* 默认固定高度 (m), 15cm, 后续可替换为VL53LXX实测值 */
#define FLOW_DT               0.01f       /* 光流任务周期 (s), 10ms */
#define DEG2RAD               0.017453292519943295f  /* PI/180 */

/*============================================================================
 * 宏定义 — VL53LXX I2C 地址
 *============================================================================*/
#define VL53LXX_I2C_ADDR      0x29        /* 7-bit 地址 (写=0x52, 读=0x53) */
#define VL53L0X_ID            0xEE        /* VL53L0X Model ID 期望值 */
#define VL53L1X_ID            0xEA        /* VL53L1X Model ID 期望值 */
#define VL53L0X_MAX_RANGE     120.0f      /* 默认模式最大距离 (cm) */
#define VL53L1X_MAX_RANGE     130.0f      /* 短距离模式最大距离 (cm) */

/*============================================================================
 * 宏定义 — VL53L0X 寄存器地址 (来源: ST VL53L0X API)
 *============================================================================*/
#define VL53L0X_REG_SYSRANGE_START              0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS         0x14
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS     0x13
#define VL53L0X_REG_I2C_STANDARD_MODE           0x88

/* 校准超时 */
#define VL53L0X_IO_TIMEOUT_MS   500U

/*============================================================================
 * 类型定义 — 12字节 Motion_Burst 结构体 (手册 motionBurst_t)
 *============================================================================*/
#pragma anon_unions
typedef __packed struct {
    __packed union {
        uint8_t motion;
        __packed struct {
            uint8_t frameFrom0    : 1;
            uint8_t runMode       : 2;
            uint8_t reserved1     : 1;
            uint8_t rawFrom0      : 1;
            uint8_t reserved2     : 2;
            uint8_t motionOccured : 1;
        };
    };
    uint8_t  observation;       /* 正常值 0xBF, 检测 EFT/B 或 ESD */
    int16_t  deltaX;            /* X 方向像素位移 */
    int16_t  deltaY;            /* Y 方向像素位移 */
    uint8_t  squal;             /* 表面质量 */
    uint8_t  rawDataSum;        /* 原始数据求和 */
    uint8_t  maxRawData;        /* 最大原始数据 */
    uint8_t  minRawData;        /* 最小原始数据 */
    uint16_t shutter;           /* 快门 (自动调节) */
} MotionBurst_t;

/*============================================================================
 * PMW3901 初始化寄存器序列
 * 来源: PixArt PMW3901MB Datasheet R1.10 (公开) + 正点原子 InitRegisters()
 *       Bank 切换通过写 0x7F, 约40条配置, 涵盖帧率/曝光/CPI/性能等
 * 谨慎修改, 错误的寄存器值可能导致传感器输出异常。
 *============================================================================*/
typedef struct {
    uint8_t addr;
    uint8_t value;
} InitRegEntry;

static const InitRegEntry init_regs[] = {
    /* --- Bank 0: 基础配置 --- */
    {0x7F, 0x00},   /* 选择 Bank 0 */
    {0x61, 0xAD},   /* 增强性能 */
    {0x7F, 0x03},   /* 选择 Bank 3 */
    {0x40, 0x00},
    {0x7F, 0x05},   /* 选择 Bank 5 */
    {0x41, 0x00},
    {0x7F, 0x06},   /* 选择 Bank 6 */
    {0x7E, 0x00},
    {0x7F, 0x07},   /* 选择 Bank 7 */
    {0x61, 0x00},
    {0x7F, 0x08},   /* 选择 Bank 8 */
    {0x64, 0x05},
    {0x6E, 0x2F},
    {0x6F, 0x00},
    /* --- Bank 5: 性能 --- */
    {0x7F, 0x05},
    {0x73, 0x00},
    /* --- Bank 0: Motion 配置 --- */
    {0x7F, 0x00},
    {0x4E, 0x11},
    /* --- Bank 14: 性能增强 --- */
    {0x7F, 0x14},
    {0x65, 0x00},
    {0x6A, 0x00},
    /* --- Bank 0: 运动检测参数 --- */
    {0x7F, 0x00},
    {0x5B, 0x00},   /* Motion_Threshold */
    {0x5A, 0xF0},
    /* --- 分辨率/CPI 设置 --- */
    {0x7F, 0x00},
    {0x21, 0x00},   /* Resolution_XY (0=no scale) */
    {0x23, 0x00},
    {0x19, 0x0F},   /* CPI_Step (affects sensitivity, max 0x77) */
    /* --- Bank 9: 帧平均 --- */
    {0x7F, 0x09},
    {0x74, 0x21},
    /* --- 回到 Bank 0 --- */
    {0x7F, 0x00},
    {0x20, 0x00},   /* Frame_Period_Max_Bound_Lower */
    {0x22, 0x00},   /* Frame_Period_Max_Bound_Upper */
    {0x18, 0x00},   /* Frame_Period_Min_Bound_Lower */
    {0x1A, 0x00},   /* Frame_Period_Min_Bound_Upper */
    /* --- 恢复 Bank 0, 发送终止 --- 不需要终止标记, 以 SIZE 遍历 */
};
#define INIT_REGS_COUNT (sizeof(init_regs) / sizeof(init_regs[0]))

/*============================================================================
 * static 内部变量 (单例封装, 对齐 IMU/I2S_beat 风格)
 *============================================================================*/

/* --- 光流数据 --- */
static OpticalFlow_Data_t s_flow_data = {{0}};

/* --- 当前帧 Motion_Burst --- */
static MotionBurst_t s_motion = {0};

/* --- 上一帧有效像素 (用于计算帧间增量) --- */
static float s_pixValidLast[2] = {0.0f, 0.0f};

/* --- 故障/异常统计 --- */
static uint32_t s_fault_cnt = 0U;
static uint32_t s_outlier_cnt = 0U;

/* --- 最近一次 Product_ID 校验结果 --- */
static uint8_t s_chip_verified = 0U;

/* --- VL53LXX --- */
static VL53LXX_Data_t s_vl53_data = {0};

/* --- VL53L0X 校准内部变量 --- */
static uint8_t  s_vl53_stopVar;                 /* 停止变量 (校准用) */
static uint32_t s_vl53_measTimingBudget_us = 30000U; /* 默认模式 30ms */
static uint8_t  s_vl53_isInitDone = 0U;         /* 初始化完成标志 */

/* --- VL53L0X 数据质量统计 (对齐手册 10帧统计+低通) --- */
static uint8_t  s_vl53_validCnt   = 0U;
static uint8_t  s_vl53_inValidCnt = 0U;
static float    s_vl53_quality    = 0.0f;

/*============================================================================
 * 私有函数声明
 *============================================================================*/
static uint8_t  PMW3901_Reg_Read(uint8_t addr);
static void     PMW3901_Reg_Write(uint8_t addr, uint8_t data);
static bool     PMW3901_ReadMotion(MotionBurst_t *out);
static bool     PMW3901_InitRegisters(void);
static void     OpticalFlow_CalcPixelComp(float *compX, float *compY);

static bool     VL53LXX_Reg_Read(uint8_t reg, uint8_t *val);
static bool     VL53LXX_Reg_Write(uint8_t reg, uint8_t val);
static bool     VL53LXX_Reg_Read16(uint16_t reg, uint8_t *val);
static uint8_t  VL53LXX_DetectType(void);

/* VL53L0X 默认模式驱动 (静态, 仅本文件使用) */
static bool     VL53L0X_WaitBoot(void);
static bool     VL53L0X_DataInit(void);
static bool     VL53L0X_StaticInit(void);
static bool     VL53L0X_PerformRefSPADManagement(void);
static bool     VL53L0X_PerformRefCalibration(void);
static bool     VL53L0X_SetSignalRateLimit(float limitMcps);
static bool     VL53L0X_SetVcselPulsePeriod(uint8_t type, uint8_t periodPclks);
static bool     VL53L0X_SetMeasTimingBudget(uint32_t budgetUs);
static bool     VL53L0X_StartContinuous(void);
static uint16_t VL53L0X_ReadRangeContinuous(void);

/*============================================================================
 * 私有函数实现 — PMW3901 SPI (寄存器读写: 2字节事务用轮询, DMA开销大无意义)
 *============================================================================*/

/**
  * @brief  读取 PMW3901 单个寄存器 (轮询, 2字节事务约13µs @1.56MHz)
  * @note   SPI Mode 3: CPOL=HIGH, CPHA=2EDGE
  *         读时序: CS↓ → MOSI发addr|0x80 → MISO收数据 → CS↑
  */
static uint8_t PMW3901_Reg_Read(uint8_t addr)
{
    uint8_t tx[2] = {addr | 0x80U, 0x00U};       /* bit7=1 表示读 */
    uint8_t rx[2] = {0U, 0U};

    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, 10U);
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

    return rx[1];
}

/**
  * @brief  写 PMW3901 单个寄存器 (轮询)
  * @note   写时序: CS↓ → MOSI发addr&0x7F+data → CS↑
  */
static void PMW3901_Reg_Write(uint8_t addr, uint8_t data)
{
    uint8_t tx[2] = {addr & 0x7FU, data};         /* bit7=0 表示写 */

    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, tx, 2, 10U);          /* 只发不收, 用 HAL_SPI_Transmit */
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  突发读取12字节 Motion_Burst (DMA模式)
  *         CS按手册要求在整个传输期间保持低电平
  * @retval true=成功, false=超时
  */
static bool PMW3901_ReadMotion(MotionBurst_t *out)
{
    uint8_t tx_buf[13] = {REG_MOTION_BURST | 0x80U}; /* bit7=1 */
    /* tx_buf[1..12] 保持 0x00 (由初始化归零) */
    uint8_t rx_buf[13] = {0U};

    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);

    if (HAL_SPI_TransmitReceive_DMA(&hspi1, tx_buf, rx_buf, 13) != HAL_OK) {
        HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
        return false;
    }

    /* 等待 DMA 完成 (13字节@1.56MHz≈67µs) */
    uint32_t timeout = 1000U;
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) {
        if (--timeout == 0U) {
            HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
            return false;
        }
    }

    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

    /* 复制结果 (跳过 rx_buf[0], 数据从 rx_buf[1] 开始12字节) */
    memcpy((uint8_t *)out, &rx_buf[1], sizeof(MotionBurst_t));
    return true;
}

/*============================================================================
 * 私有函数实现 — PMW3901 初始化
 *============================================================================*/

/**
  * @brief  写入完整的初始化寄存器序列 + 芯片ID校验
  * @retval true=芯片校验通过, false=失败
  */
static bool PMW3901_InitRegisters(void)
{
    /* 1. 上电复位 */
    PMW3901_Reg_Write(REG_POWER_UP_RESET, 0x5AU);
    HAL_Delay(50U);

    /* 2. 芯片ID快速校验 (在写完整初始化表之前, 避免芯片不在线时空转30ms) */
    uint8_t product_id     = PMW3901_Reg_Read(REG_PRODUCT_ID);
    uint8_t inv_product_id = PMW3901_Reg_Read(REG_INV_PRODUCT_ID);

    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] Product_ID=0x%02X (expect 0x49), "
                         "Inverse=0x%02X (expect 0xB6)\n",
                         product_id, inv_product_id);

    if (product_id != 0x49U || inv_product_id != 0xB6U) {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] ERROR: Chip ID mismatch!\n");
        s_chip_verified = 0U;
        return false;
    }

    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] Chip verified=%d, writing init registers...\n",
                         (int)s_chip_verified);
    s_chip_verified = 1U;

    /* 3. 写入初始化寄存器表 */
    for (uint32_t i = 0U; i < INIT_REGS_COUNT; i++) {
        PMW3901_Reg_Write(init_regs[i].addr, init_regs[i].value);
        HAL_Delay(1U);
    }
    HAL_Delay(5U);

    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] PMW3901 init complete\n");
    return true;
}

/*============================================================================
 * 私有函数实现 — 倾角补偿 (手册 getOpFlowData 公式)
 *============================================================================*/

/**
  * @brief  计算倾角补偿像素值
  *         机体倾斜导致图像"伪移动", 必须从原始像素中扣除
  *         公式: pixComp = TILT_COMP_COEF * tan(angle)
  *         系数480来自MiniFly实测, 舞蹈机器人需重新标定
  * @param  compX: [出] X方向补偿像素
  * @param  compY: [出] Y方向补偿像素
  */
static void OpticalFlow_CalcPixelComp(float *compX, float *compY)
{
    extern volatile float pitch; /* Location_deal.c */
    extern volatile float roll;  /* Location_deal.c */

    *compX = TILT_COMP_COEF * tanf(pitch * DEG2RAD);
    *compY = TILT_COMP_COEF * tanf(roll  * DEG2RAD);
}

/*============================================================================
 * 私有函数实现 — VL53LXX I2C 底层
 *============================================================================*/

/**
  * @brief  VL53LXX 8-bit寄存器读 (轮询)
  */
static bool VL53LXX_Reg_Read(uint8_t reg, uint8_t *val)
{
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(VL53LXX_I2C_ADDR << 1),
                             reg, I2C_MEMADD_SIZE_8BIT,
                             val, 1U, 50U) == HAL_OK);
}

/**
  * @brief  VL53LXX 8-bit寄存器写 (轮询)
  */
static bool VL53LXX_Reg_Write(uint8_t reg, uint8_t val)
{
    return (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(VL53LXX_I2C_ADDR << 1),
                              reg, I2C_MEMADD_SIZE_8BIT,
                              &val, 1U, 50U) == HAL_OK);
}

/**
  * @brief  VL53LXX 16-bit寄存器读 (用于VL53L1X)
  */
static bool VL53LXX_Reg_Read16(uint16_t reg, uint8_t *val)
{
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(VL53LXX_I2C_ADDR << 1),
                             reg, I2C_MEMADD_SIZE_16BIT,
                             val, 1U, 50U) == HAL_OK);
}

/**
  * @brief  VL53LXX 多字节寄存器读 (用于读取校准数据块)
  */
static bool VL53LXX_Reg_ReadMulti(uint8_t reg, uint8_t *data, uint8_t len)
{
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(VL53LXX_I2C_ADDR << 1),
                             reg, I2C_MEMADD_SIZE_8BIT,
                             data, len, 50U) == HAL_OK);
}

/**
  * @brief  VL53L0X 单字节写 (封装)
  */
static bool VL53L0X_WriteReg(uint8_t reg, uint8_t val)
{
    return VL53LXX_Reg_Write(reg, val);
}

/**
  * @brief  VL53L0X 单字节读
  */
static bool VL53L0X_ReadReg(uint8_t reg, uint8_t *val)
{
    return VL53LXX_Reg_Read(reg, val);
}

/**
  * @brief  检测 VL53LXX 传感器类型
  * @retval 0=未检测到, 1=VL53L0X, 2=VL53L1X
  */
static uint8_t VL53LXX_DetectType(void)
{
    uint8_t id = 0U;

    /* 尝试读取 VL53L0X Model ID (寄存器 0xC0) */
    if (VL53LXX_Reg_Read(0xC0U, &id) && id == VL53L0X_ID) {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] VL53L0X detected (ID=0x%02X)\n", id);
        return 1U;
    }

    /* 尝试读取 VL53L1X Model ID (16-bit 寄存器 0x010F) */
    if (VL53LXX_Reg_Read16(0x010FU, &id) && id == VL53L1X_ID) {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] VL53L1X detected (ID=0x%02X)\n", id);
        return 2U;
    }

    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] VL53LXX NOT detected (I2C addr 0x29)\n");
    return 0U;
}

/*============================================================================
 * VL53L0X 默认模式驱动 (来源: ST VL53L0X API + Pololu Arduino 移植)
 * 初始化序列: Boot → DataInit → StaticInit → SPAD校准 → 温度校准 →
 *             信号速率 → VCSEL脉冲 → 测量预算 → 启动连续测距
 * 默认模式: VCSEL PreRange=14 FinalRange=10, 预算30000µs, 范围~1.2m
 *============================================================================*/

/* --- VL53L0X 内部寄存器地址 (来自 ST API) --- */
#define VL53L0X_SYSTEM_SEQUENCE_CONFIG         0x01
#define VL53L0X_SYSTEM_RANGE_CONFIG            0x09
#define VL53L0X_SYSTEM_INTERMEASUREMENT_PERIOD 0x0C

#define VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO   0x0A
#define VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH        0x84

#define VL53L0X_SYSTEM_GROUPED_PARAMETER_HOLD  0x61

#define VL53L0X_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV  0x89
#define VL53L0X_MSRC_CONFIG_CONTROL           0x60
#define VL53L0X_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT  0x44
#define VL53L0X_DYNAMIC_SPAD_REF_EN_START_OFFSET  0x4F
#define VL53L0X_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD   0x4E
#define VL53L0X_GLOBAL_CONFIG_REF_EN_START_SELECT  0xB6
#define VL53L0X_RESULT_RANGE_VAL               0x1E  /* 16-bit 距离值 */

/* --- 内部变量 --- */
#define VCSEL_PERIOD_PRE_RANGE   0
#define VCSEL_PERIOD_FINAL_RANGE 1

/**
  * @brief  等待 VL53L0X 启动完成 (轮询寄存器 0x13 bit0)
  */
static bool VL53L0X_WaitBoot(void)
{
    uint32_t timeout = VL53L0X_IO_TIMEOUT_MS;
    uint8_t val = 0U;

    while (timeout > 0U) {
        if (!VL53L0X_ReadReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, &val)) {
            return false;
        }
        if ((val & 0x01U) == 0x01U) {
            return true;
        }
        HAL_Delay(1U);
        timeout--;
    }
    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Boot timeout!\n");
    return false;
}

/**
  * @brief  VL53L0X 数据初始化 (I2C标准模式 + 读取停止变量)
  */
static bool VL53L0X_DataInit(void)
{
    uint8_t val = 0U;

    /* I2C 标准模式 (单次写入, 非连续) */
    if (!VL53L0X_WriteReg(VL53L0X_REG_I2C_STANDARD_MODE, 0x00U)) {
        return false;
    }

    /* 读停止变量 (ST API: VL53L0X_GetStopCompletedStatus) */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_ReadReg(0x91U, &s_vl53_stopVar)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    /* 配置 GPIO (ST API: VL53L0X_SetGpioConfig) */
    if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04U)) return false;
    if (!VL53L0X_ReadReg(VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, &val)) return false;
    val &= 0xEFU;  /* 清除 bit4 */
    if (!VL53L0X_WriteReg(VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, val)) return false;

    return true;
}

/**
  * @brief  VL53L0X 静态初始化 (读取出厂校准数据 + 写入设备配置)
  *         基于 ST VL53L0X_StaticInit() 简化移植
  */
static bool VL53L0X_StaticInit(void)
{
    uint8_t val = 0U;
    uint8_t refSpadMap[6];

    /* 1. 使能 2.8V 模式 */
    if (!VL53L0X_ReadReg(VL53L0X_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, &val)) return false;
    val |= 0x01U;
    if (!VL53L0X_WriteReg(VL53L0X_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, val)) return false;

    /* 2. 读取 SPAD 信息 */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;

    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val |= 0x01U;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;

    if (!VL53L0X_WriteReg(0xFFU, 0x07U)) return false;
    if (!VL53L0X_WriteReg(0x81U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;

    /* 读取 SPAD 数量和类型 (校准触发, 值暂不使用) */
    if (!VL53L0X_ReadReg(0x94U, &val)) return false;
    if (!VL53L0X_ReadReg(0x95U, &val)) return false;

    if (!VL53L0X_WriteReg(0x81U, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;
    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val &= 0xFEU;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;

    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    /* 3. 读取参考 SPAD 映射 (6字节) */
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;

    if (!VL53LXX_Reg_ReadMulti(0xB0U, refSpadMap, 6U)) return false;

    /* 4. 写入校准配置寄存器 (来自 ST API 默认值) */
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x4FU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x4EU, 0x2CU)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xB6U, 0xB4U)) return false;

    /* 5. 使能动态 SPAD (ST API: VL53L0X_SetRefSpadChar) */
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2CU)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_GLOBAL_CONFIG_REF_EN_START_SELECT, refSpadMap[0])) return false;

    /* 6. 读取 VHV 配置和相位校准 */
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;

    /* 写系统序列配置 */
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_SEQUENCE_CONFIG, 0x01U)) return false;

    /* 执行单次测量以完成校准 */
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_REG_SYSRANGE_START, 0x01U)) return false;

    /* 等待完成 (轮询 0x13 bit0) */
    {
        uint32_t timeout = 1000U;
        while (timeout > 0U) {
            if (!VL53L0X_ReadReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, &val)) return false;
            if ((val & 0x01U) == 0x00U) break;
            HAL_Delay(1U);
            timeout--;
        }
        if (timeout == 0U) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] StaticInit measurement timeout!\n");
            return false;
        }
    }

    /* 清除中断 */
    if (!VL53L0X_WriteReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_REG_SYSRANGE_START, 0x00U)) return false;

    /* 关闭 grouped parameter hold */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_GROUPED_PARAMETER_HOLD, 0x00U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_GROUPED_PARAMETER_HOLD, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    return true;
}

/**
  * @brief  VL53L0X 执行参考 SPAD 管理校准
  */
static bool VL53L0X_PerformRefSPADManagement(void)
{
    uint8_t val = 0U;

    /* 使能 SPAD 管理 (ST API: VL53L0X_PerformRefSpadManagement) */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;

    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val |= 0x01U;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;

    /* 启动校准 */
    if (!VL53L0X_WriteReg(0xFFU, 0x07U)) return false;
    if (!VL53L0X_WriteReg(0x81U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_REG_SYSRANGE_START, 0x01U)) return false;

    /* 等待校准完成 (通常 <100ms) */
    {
        uint32_t timeout = 200U;
        while (timeout > 0U) {
            if (!VL53L0X_ReadReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, &val)) return false;
            if ((val & 0x07U) != 0U) break;
            HAL_Delay(2U);
            timeout--;
        }
        if (timeout == 0U) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] SPAD calibration timeout!\n");
        }
    }

    /* 清除中断 */
    if (!VL53L0X_WriteReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;
    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val &= 0xFEU;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    return true;
}

/**
  * @brief  VL53L0X 执行参考温度校准
  */
static bool VL53L0X_PerformRefCalibration(void)
{
    uint8_t val = 0U;

    /* 读取 VHV 配置 (ST API 校准触发序列, 必须执行) */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_ReadReg(0xCBU, &val)) return false;  /* VHV Settings (触发读取) */
    if (!VL53L0X_ReadReg(0xCCU, &val)) return false;  /* Phase Cal (触发读取) */
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;

    /* 配置并启动温度校准 */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;

    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val |= 0x01U;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;

    if (!VL53L0X_WriteReg(0xFFU, 0x07U)) return false;
    if (!VL53L0X_WriteReg(0x81U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_REG_SYSRANGE_START, 0x01U | 0x40U)) return false;

    /* 等待完成 */
    {
        uint32_t timeout = 200U;
        while (timeout > 0U) {
            if (!VL53L0X_ReadReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, &val)) return false;
            if ((val & 0x07U) != 0U) break;
            HAL_Delay(2U);
            timeout--;
        }
        if (timeout == 0U) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Temp calibration timeout!\n");
        }
    }

    /* 清除 */
    if (!VL53L0X_WriteReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;
    if (!VL53L0X_ReadReg(0x83U, &val)) return false;
    val &= 0xFEU;
    if (!VL53L0X_WriteReg(0x83U, val)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    return true;
}

/**
  * @brief  设置信号速率限制 (默认 0.25 MCPS)
  */
static bool VL53L0X_SetSignalRateLimit(float limitMcps)
{
    uint32_t tmp;

    if (limitMcps < 0.0f || limitMcps > 511.99f) return false;

    /* 计算: limitMcps * (1<<7) = 限制值 * 128 */
    tmp = (uint32_t)(limitMcps * 128.0f);
    if (tmp > 65535U) tmp = 65535U;

    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(VL53L0X_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                          (uint8_t)((tmp >> 8) & 0xFFU))) return false;
    if (!VL53L0X_WriteReg(VL53L0X_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT + 1,
                          (uint8_t)(tmp & 0xFFU))) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    return true;
}

/**
  * @brief  设置 VCSEL 脉冲周期
  * @param  type: VCSEL_PERIOD_PRE_RANGE(0) 或 VCSEL_PERIOD_FINAL_RANGE(1)
  * @param  periodPclks: 默认模式 PreRange=14, FinalRange=10
  */
static bool VL53L0X_SetVcselPulsePeriod(uint8_t type, uint8_t periodPclks)
{
    uint8_t vcselPerReg;
    uint8_t tmp;

    /* 根据类型选择起始寄存器 */
    vcselPerReg = (type == VCSEL_PERIOD_PRE_RANGE) ? 0x50U : 0x60U;

    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;

    /* 读取当前 VCSEL 周期 */
    if (!VL53L0X_ReadReg(vcselPerReg, &tmp)) return false;

    /* 写入新的 VCSEL 周期 (寄存器低8位) */
    tmp = ((tmp & 0xE0U) | (periodPclks & 0x1FU));
    if (!VL53L0X_WriteReg(vcselPerReg, tmp)) return false;

    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    return true;
}

/**
  * @brief  设置测量时间预算
  * @param  budgetUs: 预算时间 (µs), 默认 30000µs (30ms)
  *         有效范围: 20000µs ~ 等间距取决于 VCSEL 配置
  *         默认模式: PreRange=14, FinalRange=10 → 最小 ~20000µs
  */
static bool VL53L0X_SetMeasTimingBudget(uint32_t budgetUs)
{
    uint32_t usedBudgetUs;
    uint32_t finalRangeTimeoutMs;
    uint16_t macroPeriodUs;
    uint8_t  tmp;

    /* 计算 Macro Period (来自 ST API) */
    {
        uint8_t preRangePeriod, finalRangePeriod;

        if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
        if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
        if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;

        if (!VL53L0X_ReadReg(0x51U, &tmp)) return false;
        preRangePeriod = tmp & 0x1FU;
        if (!VL53L0X_ReadReg(0x61U, &tmp)) return false;
        finalRangePeriod = tmp & 0x1FU;

        if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
        if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
        if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

        /* Macro Period = 每个 VCSEL 周期时间 (来自 ST API 计算公式) */
        macroPeriodUs = (uint16_t)((uint32_t)(preRangePeriod + finalRangePeriod) * 2304U / 1000U);
    }

    /* 计算 final range timeout (基于 budget) */
    usedBudgetUs = (budgetUs > 10000U) ? (budgetUs - 10000U) : budgetUs;
    finalRangeTimeoutMs = (usedBudgetUs * 1000U) / (uint32_t)macroPeriodUs;

    /* 写 timeout 到寄存器 */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;

    /* Final Range Timeout: 16-bit, 寄存器 0x71(高) + 0x72(低) */
    if (!VL53L0X_WriteReg(0x71U, (uint8_t)((finalRangeTimeoutMs >> 8) & 0xFFU))) return false;
    if (!VL53L0X_WriteReg(0x72U, (uint8_t)(finalRangeTimeoutMs & 0xFFU))) return false;

    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    /* 保存预算值 */
    s_vl53_measTimingBudget_us = budgetUs;

    return true;
}

/**
  * @brief  启动 VL53L0X 连续测距模式
  */
static bool VL53L0X_StartContinuous(void)
{
    /* 设置连续模式 */
    if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x06U)) return false;

    /* 设置系统模式为连续测距 */
    if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_SEQUENCE_CONFIG, 0x01U)) return false;

    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x00U, 0x01U)) return false;
    if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
    if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;

    /* 设置测量间隔 (30ms = 33Hz) */
    {
        uint32_t intervalMs = (s_vl53_measTimingBudget_us + 4000U) / 1000U;
        if (intervalMs < 20U) intervalMs = 20U;

        if (!VL53L0X_WriteReg(0x80U, 0x01U)) return false;
        if (!VL53L0X_WriteReg(0xFFU, 0x01U)) return false;
        if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_INTERMEASUREMENT_PERIOD, 0x04U)) return false;
        if (!VL53L0X_WriteReg(VL53L0X_SYSTEM_INTERMEASUREMENT_PERIOD + 1,
                              (uint8_t)(intervalMs & 0xFFU))) return false;
        if (!VL53L0X_WriteReg(0xFFU, 0x00U)) return false;
        if (!VL53L0X_WriteReg(0x80U, 0x00U)) return false;
    }

    /* 启动连续测量 */
    if (!VL53L0X_WriteReg(VL53L0X_REG_SYSRANGE_START, 0x02U)) return false;

    return true;
}

/**
  * @brief  读取连续测距结果 (非阻塞: 若无新数据则返回0)
  * @retval 距离值 (mm), 0=数据未就绪或无效
  */
static uint16_t VL53L0X_ReadRangeContinuous(void)
{
    uint8_t val = 0U;
    uint16_t distance = 0U;

    /* 检查数据就绪 (寄存器 0x13 bit0) */
    if (!VL53L0X_ReadReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, &val)) {
        return 0U;
    }

    if ((val & 0x01U) == 0U) {
        return 0U; /* 数据未就绪 */
    }

    /* 读取 16-bit 距离值 (mm), 寄存器 0x1E(高) + 0x1F(低) */
    uint8_t buf[2] = {0U, 0U};
    if (!VL53LXX_Reg_ReadMulti(VL53L0X_RESULT_RANGE_VAL, buf, 2U)) {
        return 0U;
    }

    distance = ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];

    /* 清除中断 */
    VL53L0X_WriteReg(VL53L0X_REG_RESULT_INTERRUPT_STATUS, 0x01U);

    return distance;
}

/*============================================================================
 * 公开函数实现 — OpticalFlow_Init
 *============================================================================*/

void OpticalFlow_Init(void)
{
    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] Init start...\n");

    /* 清零所有数据结构 */
    memset(&s_flow_data,  0, sizeof(s_flow_data));
    memset(&s_motion,     0, sizeof(s_motion));
    memset(&s_vl53_data,  0, sizeof(s_vl53_data));
    s_pixValidLast[0] = 0.0f;
    s_pixValidLast[1] = 0.0f;
    s_fault_cnt   = 0U;
    s_outlier_cnt = 0U;

    /* CS 拉高 (PA4 已在 gpio.c 初始化为推挽输出 High) */
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

    /* PMW3901 初始化 + 芯片校验 */
    if (PMW3901_InitRegisters()) {
        s_flow_data.isFlowOk  = 1U;
        s_flow_data.isDataValid = 1U;
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] PMW3901 init OK\n");
    } else {
        s_flow_data.isFlowOk  = 0U;
        s_flow_data.isDataValid = 0U;
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] PMW3901 init FAILED\n");
    }

    /* VL53LXX 初始化 */
    VL53LXX_Init();

    (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] Init done (flow=%d, vl53=%d)\n",
                         s_flow_data.isFlowOk, s_vl53_data.sensorType);
}

/*============================================================================
 * 公开函数实现 — OpticalFlow_Process (100Hz, 挂 task_table)
 *============================================================================*/

void OpticalFlow_Process(void)
{
    /* 光流芯片异常或未初始化, 跳过 */
    if (!s_flow_data.isFlowOk) {
        return;
    }

    /* --- 0. 更新 VL53L0X 激光测距数据 (为第7步高度计算准备) --- */
    VL53LXX_Data_Update();

    /* --- 1. DMA 突发读取 12 字节运动数据 --- */
    if (!PMW3901_ReadMotion(&s_motion)) {
        return; /* DMA 超时, 跳过本帧 */
    }

    /* --- 2. 故障检测 (手册: minRawData==0&&maxRawData==0 连续1秒) --- */
    if (s_motion.minRawData == 0U && s_motion.maxRawData == 0U) {
        s_fault_cnt++;
        if (s_fault_cnt >= FAULT_THRESHOLD) {
            s_flow_data.isFlowOk = 0U;
            s_flow_data.isDataValid = 0U;
            SEGGER_RTT_printf(0, "[OpticalFlow] Fault detected (100 frames no data)\n");
        }
        return;
    }
    s_fault_cnt = 0U;

    /* --- 3. 轴映射 (手册28页: pitch→X, roll→Y; 地面参考系取反) --- */
    int16_t pixelDx =  s_motion.deltaY;      /* pitch 方向 → X */
    int16_t pixelDy = -s_motion.deltaX;      /* roll 方向 → Y (取反) */

    /* --- 4. 异常值限幅 (单帧像素变化过大则丢弃) --- */
    int32_t absDx = (pixelDx >= 0) ? pixelDx : -pixelDx;
    int32_t absDy = (pixelDy >= 0) ? pixelDy : -pixelDy;
    if (absDx > OUTLIER_LIMIT || absDy > OUTLIER_LIMIT) {
        s_outlier_cnt++;
        return;
    }
    s_outlier_cnt = 0U;

    /* --- 5. 倾角补偿 --- */
    float pixCompX = 0.0f, pixCompY = 0.0f;
    OpticalFlow_CalcPixelComp(&pixCompX, &pixCompY);

    /* --- 6. 累积原始像素 + 倾角补偿 → 有效像素 --- */
    s_flow_data.pixSum[0]    += (float)pixelDx;
    s_flow_data.pixSum[1]    += (float)pixelDy;
    s_flow_data.pixValid[0]   = s_flow_data.pixSum[0] + pixCompX;
    s_flow_data.pixValid[1]   = s_flow_data.pixSum[1] + pixCompY;

    /* --- 7. 高度系数 (优先使用 VL53L0X 实测高度, 失败回退默认值) --- */
    float height_m = DEFAULT_HEIGHT_M;
    if (s_vl53_data.isDataValid && s_vl53_data.distance > 0.0f) {
        height_m = s_vl53_data.distance * 0.01f;  /* cm → m */
    }
    if (height_m < HEIGHT_MIN) {
        height_m = 0.0f;   /* <5cm 光流失效 */
    } else if (height_m > HEIGHT_MAX) {
        height_m = HEIGHT_MAX;
    }
    float coeff = RESOLUTION * height_m;  /* 像素→cm 换算系数 */

    /* --- 8. 帧间位移 (cm) --- */
    s_flow_data.deltaPos[0] = coeff * (s_flow_data.pixValid[0] - s_pixValidLast[0]);
    s_flow_data.deltaPos[1] = coeff * (s_flow_data.pixValid[1] - s_pixValidLast[1]);
    s_pixValidLast[0] = s_flow_data.pixValid[0];
    s_pixValidLast[1] = s_flow_data.pixValid[1];

    /* --- 9. 瞬时速度 (cm/s) --- */
    s_flow_data.deltaVel[0] = s_flow_data.deltaPos[0] / FLOW_DT;
    s_flow_data.deltaVel[1] = s_flow_data.deltaPos[1] / FLOW_DT;

    /* --- 10. 速度低通滤波 (0.15) + 限幅 --- */
    s_flow_data.velLpf[0] += (s_flow_data.deltaVel[0] - s_flow_data.velLpf[0]) * VEL_LPF_COEF;
    s_flow_data.velLpf[1] += (s_flow_data.deltaVel[1] - s_flow_data.velLpf[1]) * VEL_LPF_COEF;

    s_flow_data.velLpf[0] = (s_flow_data.velLpf[0] >  VEL_LIMIT) ?  VEL_LIMIT :
                            (s_flow_data.velLpf[0] < -VEL_LIMIT) ? -VEL_LIMIT :
                             s_flow_data.velLpf[0];
    s_flow_data.velLpf[1] = (s_flow_data.velLpf[1] >  VEL_LIMIT) ?  VEL_LIMIT :
                            (s_flow_data.velLpf[1] < -VEL_LIMIT) ? -VEL_LIMIT :
                             s_flow_data.velLpf[1];

    /* --- 11. 累积位移积分 (cm) --- */
    s_flow_data.posSum[0] += s_flow_data.deltaPos[0];
    s_flow_data.posSum[1] += s_flow_data.deltaPos[1];

    s_flow_data.isDataValid = 1U;
}

/*============================================================================
 * 公开函数实现 — 数据接口
 *============================================================================*/

OpticalFlow_Data_t* OpticalFlow_GetData(void)
{
    return &s_flow_data;
}

void OpticalFlow_ResetPosSum(void)
{
    s_flow_data.posSum[0] = 0.0f;
    s_flow_data.posSum[1] = 0.0f;
    s_pixValidLast[0] = s_flow_data.pixValid[0];
    s_pixValidLast[1] = s_flow_data.pixValid[1];
}

/*============================================================================
 * 公开函数实现 — VL53LXX (完整默认模式初始化 + 数据更新)
 *============================================================================*/

/**
  * @brief  VL53LXX 初始化 (设备检测 + VL53L0X 默认模式完整校准)
  */
void VL53LXX_Init(void)
{
    s_vl53_data.sensorType = VL53LXX_DetectType();

    if (s_vl53_data.sensorType == 1U) {
        /* --- VL53L0X 默认模式完整初始化 --- */
        if (!VL53L0X_WaitBoot()) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] WaitBoot FAILED\n");
            s_vl53_data.isDataValid = 0U;
            return;
        }
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Boot OK\n");

        if (!VL53L0X_DataInit()) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] DataInit FAILED\n");
            s_vl53_data.isDataValid = 0U;
            return;
        }
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] DataInit OK\n");

        if (!VL53L0X_StaticInit()) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] StaticInit FAILED\n");
            s_vl53_data.isDataValid = 0U;
            return;
        }
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] StaticInit OK\n");

        /* 执行 SPAD 管理和温度校准 (不可跳过, 否则测距不准) */
        VL53L0X_PerformRefSPADManagement();
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] SPAD cal done\n");

        VL53L0X_PerformRefCalibration();
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Temp cal done\n");

        /* 设置测距参数 (默认模式) */
        VL53L0X_SetSignalRateLimit(0.25f);
        VL53L0X_SetVcselPulsePeriod(VCSEL_PERIOD_PRE_RANGE,   14U);
        VL53L0X_SetVcselPulsePeriod(VCSEL_PERIOD_FINAL_RANGE, 10U);
        VL53L0X_SetMeasTimingBudget(30000U);

        /* 启动连续测距 */
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Starting continuous measurement...\n");
        if (!VL53L0X_StartContinuous()) {
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] StartContinuous FAILED\n");
            s_vl53_data.isDataValid = 0U;
            return;
        }

        s_vl53_isInitDone = 1U;
        s_vl53_data.isDataValid = 1U;
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Init OK, budget=%lu us\n",
                          s_vl53_measTimingBudget_us);
    }
    else if (s_vl53_data.sensorType == 2U) {
        /*
         * TODO: VL53L1X 短距离模式初始化
         * 参考: ST VL53L1X API vl53l1xSetParam()
         * VL53L1_SetDistanceMode(VL53L1_DISTANCEMODE_SHORT)
         * VL53L1_SetMeasurementTimingBudgetMicroSeconds(20000)
         * VL53L1_StartMeasurement()
         */
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[OpticalFlow] VL53L1X basic init (full calibration TODO)\n");
    }
    else {
        s_vl53_data.isDataValid = 0U;
    }
}

/**
  * @brief  VL53L0X 周期数据更新 (在 OpticalFlow_Process 100Hz 中调用)
  *         读取连续测距结果 + 质量统计低通 (对齐手册 10 帧统计逻辑)
  * @note   VL53L0X 测量速率 ~33Hz (30ms预算), 本函数 100Hz 调用,
  *         仅在新数据就绪时才更新计数器, 避免空读污染质量统计。
  */
void VL53LXX_Data_Update(void)
{
    if (!s_vl53_isInitDone || s_vl53_data.sensorType != 1U) {
        return;
    }

    uint16_t range_mm = VL53L0X_ReadRangeContinuous();

    /* 无新数据 → 保持上一次状态不变 (传感器仍在测量中) */
    if (range_mm == 0U) {
        return;
    }

    float range_cm = (float)range_mm * 0.1f;

    /* --- 掉线检测 (手册: range>=6550 表示传感器掉线/I2C故障) --- */
    static uint8_t s_dropout_cnt = 0U;
    if (range_mm >= 6550U) {
        s_dropout_cnt++;
        if (s_dropout_cnt >= 30U) {
            s_vl53_data.isDataValid = 0U;
            s_vl53_isInitDone = 0U;
            s_dropout_cnt = 0U;
            (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[VL53L0X] Sensor dropout detected!\n");
        }
        return;
    }
    s_dropout_cnt = 0U;

    /* 有效性判断 + 10帧统计低通 (对齐手册 vl53l0xTask 逻辑) */
    if (range_cm < VL53L0X_MAX_RANGE) {
        s_vl53_data.distance = range_cm;
        s_vl53_validCnt++;
    } else {
        s_vl53_inValidCnt++;
    }

    if (s_vl53_validCnt + s_vl53_inValidCnt >= 10U) {
        s_vl53_quality += ((float)s_vl53_validCnt / 10.0f - s_vl53_quality) * 0.1f;
        s_vl53_data.quality = s_vl53_quality;
        s_vl53_validCnt   = 0U;
        s_vl53_inValidCnt = 0U;
    }

    /* 数据可用标志: 以质量+最新距离综合判断 (对齐手册 vl53lxxReadRange) */
    s_vl53_data.isDataValid = (range_cm > 0.0f && range_cm < VL53L0X_MAX_RANGE) ? 1U : 0U;
}

VL53LXX_Data_t* VL53LXX_GetData(void)
{
    return &s_vl53_data;
}
