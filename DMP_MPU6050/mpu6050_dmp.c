/*============================================================================
 * mpu6050_dmp.c — MPU6050 DMP 高层封装
 *
 * 基于 InvenSense Motion Driver v5.1.3 (inv_mpu + inv_mpu_dmp_motion_driver)
 * 提供简洁的初始化与姿态读取接口。
 *
 * DMP 初始化流程:
 *   mpu_init() → 配置传感器 → 加载 DMP 固件 → 使能 DMP → 使能 FIFO
 *
 * 姿态读取:
 *   dmp_read_fifo() → 四元数 → 欧拉角 (Yaw/Pitch/Roll)
 *============================================================================*/

#include "mpu6050_dmp.h"
#include "stm32_mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <math.h>

/*============================================================================*
 * I2C 句柄弱符号 — CubeMX 强符号自动覆盖
 * 未配置的外设保持零初始化 (Instance == NULL, 检测时跳过)
 *============================================================================*/
__weak I2C_HandleTypeDef hi2c1 = {0};
__weak I2C_HandleTypeDef hi2c2 = {0};
__weak I2C_HandleTypeDef hi2c3 = {0};

/* 运行时 I2C 句柄指针（自动检测赋值） */
I2C_HandleTypeDef *g_mpu_i2c = NULL;

/* 自动检测结果（供总线恢复等内部函数使用） */
static int      g_detected_periph = 0;    /* 1=I2C1, 2=I2C2, 3=I2C3 */
uint8_t         g_detected_addr   = 0;    /* 0x68 或 0x69 (extern 可见) */

#define PI                  3.14159265358979f
#define RAD_TO_DEG          57.2957795130823f

/* DMP 输出速率 (Hz) */
#define DMP_SAMPLE_RATE     100

/* 前向声明 */
static int MPU6050_DetectI2C(void);

/*--------------------------------------------------------------------------*
 * 初始化 MPU6050 并加载 DMP 固件
 * 返回: 0 = 成功, 其它 = 失败
 *--------------------------------------------------------------------------*/
int MPU6050_DMP_Init(void)
{
    int ret;

    /* ---- 步骤 0: 自动检测 I2C 总线及 MPU6050 地址 ---- */
    ret = MPU6050_DetectI2C();
    if (ret != 0)
        return -4;

    /* ---- 步骤 1: 复位并初始化传感器 ---- */
    /* (inv_mpu 内部会再次探测地址, g_mpu_i2c 已就绪即可) */
    ret = mpu_init();
    if (ret != 0)
        return -1;

    /* ---- 步骤 2: 设置传感器使能 ---- */
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    /* ---- 步骤 3: 配置 FIFO ---- */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    /* ---- 步骤 4: 设置采样率 ---- */
    mpu_set_sample_rate(DMP_SAMPLE_RATE);

    /* ---- 步骤 5: 加载 DMP 固件 ---- */
    ret = dmp_load_motion_driver_firmware();
    if (ret != 0)
        return -2;

    /* ---- 步骤 6: 配置 DMP 输出速率 ---- */
    dmp_set_fifo_rate(DMP_SAMPLE_RATE);

    /* ---- 步骤 7: 使能 DMP 功能 ---- */
    /*   DMP_FEATURE_6X_LP_QUAT  : 6 轴低功耗四元数输出
     *   DMP_FEATURE_GYRO_CAL    : 陀螺仪自动校准（无运动 4 秒后启动）
     *   DMP_FEATURE_SEND_RAW_ACCEL : 同时输出原始加速度
     */
    dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL | DMP_FEATURE_SEND_RAW_ACCEL);

    /* ---- 步骤 7.5: GYRO_CAL 无运动超时阈值: 8 秒 → 4 秒 ---- */
    /* DMP 内部定时器 tick = 1000 / DMP_SAMPLE_RATE = 10ms, 4s = 4000ms → 400 ticks */
    {
        unsigned short ticks = 4000 / (1000 / DMP_SAMPLE_RATE);
        unsigned char val;
        val = (ticks >> 8) & 0xFF;
        mpu_write_mem(360, 1, &val);   /* DMP_MNMTMTHRH = 360 (高位) */
        val = ticks & 0xFF;
        mpu_write_mem(362, 1, &val);   /* DMP_MNMTMTHRL = 362 (低位) */
    }

    /* ---- 步骤 8: 设置中断模式为连续 ---- */
    dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);

    mpu_set_int_level(0);      // 高电平有效（默认）
    mpu_set_int_latched(1);    // 锁存模式，读取 MPU 寄存器后自动清除中断

    /* ---- 步骤 9: 使能 DMP ---- */
    ret = mpu_set_dmp_state(1);
    if (ret != 0)
        return -3;

    return 0;
}

/*--------------------------------------------------------------------------*
 * 读取 DMP 四元数
 * 输出: qw, qx, qy, qz (定点数, 缩放因子 2^30)
 * 返回: 0 = 有新数据, 非 0 = 无数据
 *--------------------------------------------------------------------------*/
int MPU6050_DMP_GetQuaternion(long *qw, long *qx, long *qy, long *qz)
{
    short gyro[3], accel[3], sensors;
    long quat[4];
    unsigned char more;
    unsigned long timestamp;

    /* 从 FIFO 读一组数据 */
    int ret = dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more);
    if (ret != 0)
        return -1;

    /* dmp_read_fifo 成功 = 四元数数据已读取 (DMP_FEATURE_6X_LP_QUAT) */
    if (qw) *qw = quat[0];
    if (qx) *qx = quat[1];
    if (qy) *qy = quat[2];
    if (qz) *qz = quat[3];

    return 0;
}

/*--------------------------------------------------------------------------*
 * 将 DMP 四元数 (定点 Q30) 转换为欧拉角 (度)
 * 输出: pitch (俯仰), roll (横滚), yaw (偏航)
 * 返回: 0 = 成功, 非 0 = 无数据
 *--------------------------------------------------------------------------*/
int MPU6050_DMP_GetEuler(float *pitch, float *roll, float *yaw)
{
    long quat[4];
    int ret = MPU6050_DMP_GetQuaternion(&quat[0], &quat[1],
                                        &quat[2], &quat[3]);
    if (ret != 0)
        return ret;

    /* 将 Q30 定点数转换为浮点数 */
    float qw = (float)quat[0] / 1073741824.0f;   /* 2^30 */
    float qx = (float)quat[1] / 1073741824.0f;
    float qy = (float)quat[2] / 1073741824.0f;
    float qz = (float)quat[3] / 1073741824.0f;

    /* 四元数 → 欧拉角 (参考航空顺序: Z-Y-X, 即 Yaw-Pitch-Roll) */
    float sqw = qw * qw;
    float sqx = qx * qx;
    float sqy = qy * qy;
    float sqz = qz * qz;

    /* Roll (x轴旋转) */
    float roll_val = atan2f(2.0f * (qy * qz + qw * qx),
                            sqw - sqx - sqy + sqz);

    /* Pitch (y轴旋转) */
    float sinp = -2.0f * (qx * qz - qw * qy);
    float pitch_val;
    if (sinp > 1.0f)
        pitch_val = PI / 2.0f;
    else if (sinp < -1.0f)
        pitch_val = -PI / 2.0f;
    else
        pitch_val = asinf(sinp);

    /* Yaw (z轴旋转) */
    float yaw_val = atan2f(2.0f * (qx * qy + qw * qz),
                           sqw + sqx - sqy - sqz);

    if (pitch) *pitch = pitch_val * RAD_TO_DEG;
    if (roll)  *roll  = roll_val  * RAD_TO_DEG;
    if (yaw)   *yaw   = yaw_val   * RAD_TO_DEG;

    return 0;
}

/*============================================================================*
 * I2C 引脚检测 — 自动识别任意 I2C 外设 (1/2/3) 的 SCL/SDA 引脚
 *
 * 扫描 GPIO AFR 寄存器, 匹配所有已知的 F4 引脚映射:  
 *   I2C1: PB6/PB7, PB8/PB9    I2C2: PB10/PB11    I2C3: PA8/PC9
 *============================================================================*/
static int I2C_FindPins(int periph_num,
                         GPIO_TypeDef **scl_port, uint16_t *scl_pin,
                         GPIO_TypeDef **sda_port, uint16_t *sda_pin)
{
    static const struct {
        int         periph;
        GPIO_TypeDef *port;
        uint8_t     pin;       /* 引脚号 0～15 */
        int         is_scl;    /* 1=SCL, 0=SDA */
    } tbl[] = {
        /* I2C1 */
        {1, GPIOB,  6, 1}, {1, GPIOB,  7, 0},   /* PB6(SCL) PB7(SDA) */
        {1, GPIOB,  8, 1}, {1, GPIOB,  9, 0},   /* PB8(SCL) PB9(SDA) */
        /* I2C2 */
        {2, GPIOB, 10, 1}, {2, GPIOB, 11, 0},   /* PB10(SCL) PB11(SDA) */
        /* I2C3 */
        {3, GPIOA,  8, 1}, {3, GPIOC,  9, 0},   /* PA8(SCL) PC9(SDA)  */
    };

    *scl_port = NULL;
    *sda_port = NULL;

    for (int i = 0; i < (int)(sizeof(tbl) / sizeof(tbl[0])); i++) {
        if (tbl[i].periph != periph_num)
            continue;

        int afr_shift  = (tbl[i].pin % 8) * 4;
        int afr_idx    = tbl[i].pin / 8;
        uint32_t afr   = tbl[i].port->AFR[afr_idx];
        uint32_t moder = tbl[i].port->MODER;

        /* AF4 (I2C) + 复用功能模式(0b10) */
        if (((afr >> afr_shift) & 0xF) == MPU6050_I2C1_AF &&
            ((moder >> (tbl[i].pin * 2)) & 0x3) == 2) {
            if (tbl[i].is_scl) {
                *scl_port = tbl[i].port;
                *scl_pin  = (uint16_t)(1 << tbl[i].pin);
            } else {
                *sda_port = tbl[i].port;
                *sda_pin  = (uint16_t)(1 << tbl[i].pin);
            }
        }
    }

    return (*scl_port && *sda_port) ? 0 : -1;
}

/*============================================================================*
 * I2C 自动检测 — 扫描所有 I2C 总线, 探测 MPU6050 设备
 *
 * 1. 遍历 hi2c1/hi2c2/hi2c3（CubeMX 未配置的外设 Instance==NULL 自动跳过）
 * 2. 检查 RCC 时钟使能位
 * 3. 尝试 0x68 / 0x69 两个地址
 * 4. 成功时填充 g_mpu_i2c / g_detected_periph / g_detected_addr
 *============================================================================*/
static int MPU6050_DetectI2C(void)
{
    /* 与 __weak 句柄声明顺序一致: 0→hi2c1, 1→hi2c2, 2→hi2c3 */
    I2C_HandleTypeDef *handles[] = {&hi2c1, &hi2c2, &hi2c3};
    uint8_t addrs[] = {0x68, 0x69};

    /* RCC->APB1ENR 位: I2C1=21, I2C2=22, I2C3=23 */
    static const uint8_t rcc_bit[] = {21, 22, 23};

    for (int i = 0; i < 3; i++) {
        /* 跳过：外设未配置 or 时钟未使能 */
        if (handles[i]->Instance == NULL)       continue;
        if (!(RCC->APB1ENR & (1 << rcc_bit[i]))) continue;

        for (int j = 0; j < 2; j++) {
            if (HAL_I2C_IsDeviceReady(handles[i],
                    (uint16_t)addrs[j] << 1, 3, 50) == HAL_OK) {
                g_mpu_i2c          = handles[i];
                g_detected_periph  = i + 1;
                g_detected_addr    = addrs[j];
                return 0;
            }
        }
    }

    return -1;
}

/*============================================================================*
 * I2C 总线恢复 — 手动产生 SCL 脉冲释放被锁死的 SDA
 *============================================================================*/
int MPU6050_I2C_BusRecovery(void)
{
    GPIO_TypeDef *scl_port = NULL, *sda_port = NULL;
    uint16_t scl_pin = 0, sda_pin = 0;
    GPIO_InitTypeDef gpio = {0};

    /* 如果尚未自动检测, 先执行检测 */
    if (g_detected_periph == 0) {
        if (MPU6050_DetectI2C() != 0)
            return -1;
    }

    /* 1. 查找当前 I2C 外设的 SCL/SDA 引脚 */
    if (I2C_FindPins(g_detected_periph,
                     &scl_port, &scl_pin, &sda_port, &sda_pin) != 0)
        return -1;

    /* 2. 关闭 I2C 外设 */
    HAL_I2C_DeInit(g_mpu_i2c);

    /* 3. SCL/SDA → GPIO 推挽输出 */
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Pin   = scl_pin;
    HAL_GPIO_Init(scl_port, &gpio);
    gpio.Pin   = sda_pin;
    HAL_GPIO_Init(sda_port, &gpio);

    /* 4. 先拉高 SDA 和 SCL */
    HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(sda_port, sda_pin, GPIO_PIN_SET);
    HAL_Delay(1);

    /* 5. 9 个 SCL 时钟脉冲：让从机释放 SDA */
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    /* 6. STOP 条件：SDA↓ → SCL↑ → SDA↑ */
    HAL_GPIO_WritePin(sda_port, sda_pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(sda_port, sda_pin, GPIO_PIN_SET);
    HAL_Delay(1);

    /* 7. 恢复 SCL/SDA 为 I2C 复用开漏 */
    gpio.Mode      = GPIO_MODE_AF_OD;
    gpio.Alternate = MPU6050_I2C1_AF;
    gpio.Pin       = scl_pin;
    HAL_GPIO_Init(scl_port, &gpio);
    gpio.Pin       = sda_pin;
    HAL_GPIO_Init(sda_port, &gpio);

    return 0;   /* 用户需在外部调用 MX_I2Cx_Init() 完成 I2C 重新初始化 */
}
