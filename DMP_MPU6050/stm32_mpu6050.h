#ifndef _STM32_MPU6050_H_
#define _STM32_MPU6050_H_

/*============================================================================
 * stm32_mpu6050.h — STM32 HAL 移植适配层
 *
 * 将 inv_mpu.c / inv_mpu_dmp_motion_driver.c 所需的平台函数
 * 映射到 STM32 HAL (I2C + SysTick)
 *
 * 使用前需要在编译选项中定义:
 *   MPU6050, EMPL
 *============================================================================*/

#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <math.h>

/*--------------------------------------------------------------------------*
 * I2C 读写 — 运行时 I2C 句柄指针（由 mpu6050_dmp.c 的自动检测赋值）
 *--------------------------------------------------------------------------*/
extern I2C_HandleTypeDef *g_mpu_i2c;

static inline int MPU6050_I2C_WriteRegister(unsigned char addr,
                                            unsigned char reg,
                                            unsigned char len,
                                            const unsigned char *data)
{
    if (!g_mpu_i2c) return -1;
    return (HAL_I2C_Mem_Write(g_mpu_i2c, (uint16_t)(addr << 1), reg,
                              I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len,
                              100) == HAL_OK) ? 0 : -1;
}

static inline int MPU6050_I2C_ReadRegister(unsigned char addr,
                                           unsigned char reg,
                                           unsigned char len,
                                           unsigned char *data)
{
    if (!g_mpu_i2c) return -1;
    return (HAL_I2C_Mem_Read(g_mpu_i2c, (uint16_t)(addr << 1), reg,
                             I2C_MEMADD_SIZE_8BIT, data, len,
                             100) == HAL_OK) ? 0 : -1;
}

/*--------------------------------------------------------------------------*
 * inv_mpu.c 平台函数宏映射
 *--------------------------------------------------------------------------*/
#define i2c_write    MPU6050_I2C_WriteRegister
#define i2c_read     MPU6050_I2C_ReadRegister
#define delay_ms     HAL_Delay
#define get_ms(x)    do { *(x) = HAL_GetTick(); } while(0)
#define min(a,b)     (((a) < (b)) ? (a) : (b))

/* labs() 和 fabsf() 由标准库提供，无需额外定义 */

/*--------------------------------------------------------------------------*
 * I2C 总线恢复 — GPIO 复用功能编号（不同 STM32 系列 AF 号不同）
 *   F0: AF1,  F1: 无 AF 系统,  F4/F7/H7: AF4
 *--------------------------------------------------------------------------*/
#ifndef MPU6050_I2C1_AF
  #define MPU6050_I2C1_AF  GPIO_AF4_I2C1
#endif

#endif /* _STM32_MPU6050_H_ */
