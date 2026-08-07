/**
  ******************************************************************************
  * @file    Optical_flow.h
  * @brief   ATK-PMW3901 光流模块驱动头文件
  *          - PMW3901 光流传感器: SPI1 + DMA (PA5/6/7, PA4=CS, 1.56MHz Mode3)
  *          - VL53LXX 激光测距:  I2C1 轮询 (PB6/PB7, 400kHz, 默认模式)
  *          100Hz 光流任务挂入 task_table 合作式调度器
  ******************************************************************************
  */

#ifndef __OPTICAL_FLOW_H
#define __OPTICAL_FLOW_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* 光流数据结构 ---------------------------------------------------------------*/

/** @brief 光流传感器数据 (全部 float, 单位 cm 或 cm/s) */
typedef struct {
    float deltaPos[2];      /* 两帧之间位移, 单位 cm: [0]=X方向, [1]=Y方向 */
    float deltaVel[2];      /* 瞬时速度, 单位 cm/s                              */
    float posSum[2];        /* 累积位移(自起飞/复位), 单位 cm                   */
    float velLpf[2];        /* 低通后速度, 单位 cm/s                             */
    float pixSum[2];        /* 累积原始像素                                     */
    float pixValid[2];      /* 经倾角补偿后的有效像素                           */
    uint8_t isFlowOk;       /* 光流芯片工作正常标志                             */
    uint8_t isDataValid;    /* 当前帧数据有效 (在高度/故障范围内)                */
} OpticalFlow_Data_t;

/** @brief VL53LXX 激光传感器数据 */
typedef struct {
    float distance;         /* 测量距离, 单位 cm                                */
    float quality;          /* 数据可信度 0.0~1.0                               */
    uint8_t isDataValid;    /* 距离数据有效标志                                  */
    uint8_t sensorType;     /* 传感器类型: 0=无, 1=VL53L0X, 2=VL53L1X           */
} VL53LXX_Data_t;

/* 函数声明 ------------------------------------------------------------------*/

/**
  * @brief  光流模块初始化
  *         - 上电复位 + 芯片ID校验 (Product_ID=0x49, Inverse=0xB6)
  *         - 写入初始化寄存器序列
  *         - 初始化 VL53LXX 激光传感器 (默认模式)
  * @note   必须在 MX_SPI1_Init() 和 MX_I2C1_Init() 之后调用
  * @retval 无
  */
void OpticalFlow_Init(void);

/**
  * @brief  光流处理任务 (100Hz, 10ms 周期)
  *         - DMA 突发读取 12 字节 Motion_Burst
  *         - 异常/故障检测 + 轴映射 + 异常值限幅
  *         - 倾角补偿 (复用 Location_deal 的 pitch/roll)
  *         - 像素→位移换算 + 速度微分 + 低通滤波 + 限幅
  * @note   需挂入 Task.c 的 task_table (周期 10ms)
  * @retval 无
  */
void OpticalFlow_Process(void);

/**
  * @brief  获取光流数据指针
  * @retval OpticalFlow_Data_t 结构体指针
  */
OpticalFlow_Data_t* OpticalFlow_GetData(void);

/**
  * @brief  累积位移清零 (动作编排起跑点 / 起飞前调用)
  * @retval 无
  */
void OpticalFlow_ResetPosSum(void);

/**
  * @brief  VL53LXX 激光传感器初始化
  *         - I2C1 扫描设备地址 0x29
  *         - 自动识别 VL53L0X 或 VL53L1X
  *         - 配置默认模式 (120cm / 30ms)
  * @note   由 OpticalFlow_Init() 内部调用, 也可单独调用
  * @retval 无
  */
void VL53LXX_Init(void);

/**
  * @brief  获取激光测距数据指针
  * @retval VL53LXX_Data_t 结构体指针
  */
VL53LXX_Data_t* VL53LXX_GetData(void);

/**
  * @brief  VL53L0X 周期数据更新 (在光流任务中调用, 刷新距离/质量)
  *         - 读取连续测距结果
  *         - 10帧统计 + 低通质量计算
  *         - 由 OpticalFlow_Process() 内部自动调用
  * @retval 无
  */
void VL53LXX_Data_Update(void);

#endif /* __OPTICAL_FLOW_H */
