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
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <math.h>

#define PI                  3.14159265358979f
#define RAD_TO_DEG          57.2957795130823f

/* DMP 输出速率 (Hz) */
#define DMP_SAMPLE_RATE     50

/*--------------------------------------------------------------------------*
 * 初始化 MPU6050 并加载 DMP 固件
 * 返回: 0 = 成功, 其它 = 失败
 *--------------------------------------------------------------------------*/
int MPU6050_DMP_Init(void)
{
    int ret;

    /* ---- 步骤 1: 复位并初始化传感器 ---- */
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
     *   DMP_FEATURE_GYRO_CAL    : 陀螺仪自动校准
     *   DMP_FEATURE_SEND_RAW_ACCEL : 同时输出原始加速度
     */
    dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL);

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
