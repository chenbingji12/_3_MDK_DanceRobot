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
 * I2C 读写 — 映射到 STM32 HAL 硬件 I2C
 *--------------------------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

static inline int MPU6050_I2C_WriteRegister(unsigned char addr,
                                            unsigned char reg,
                                            unsigned char len,
                                            const unsigned char *data)
{
    return (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(addr << 1), reg,
                              I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len,
                              100) == HAL_OK) ? 0 : -1;
}

static inline int MPU6050_I2C_ReadRegister(unsigned char addr,
                                           unsigned char reg,
                                           unsigned char len,
                                           unsigned char *data)
{
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), reg,
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

#endif /* _STM32_MPU6050_H_ */
