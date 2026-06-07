#ifndef _MPU6050_DMP_H_
#define _MPU6050_DMP_H_

#include <stdint.h>

/* DMP 初始化: 返回 0 成功, 非 0 失败 */
int MPU6050_DMP_Init(void);

/* 读取 DMP 欧拉角 (度), 返回 0 成功, 非 0 表示数据尚未就绪 */
int MPU6050_DMP_GetEuler(float *pitch, float *roll, float *yaw);

/* 读取原始四元数 [w, x, y, z], 返回 0 成功 */
int MPU6050_DMP_GetQuaternion(long *qw, long *qx, long *qy, long *qz);

/* I2C 总线恢复: 自动检测 I2C1 SCL/SDA 引脚, 手动发 9 个 SCL 脉冲释放总线
 * 返回 0 成功, -1 未检测到 I2C1 引脚
 * 注意: 调用后需由外部重新初始化 I2C (MX_I2C1_Init())
 */
int MPU6050_I2C_BusRecovery(void);

#endif /* _MPU6050_DMP_H_ */
