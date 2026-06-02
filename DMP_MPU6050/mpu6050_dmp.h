#ifndef _MPU6050_DMP_H_
#define _MPU6050_DMP_H_

#include <stdint.h>

/* DMP 初始化: 返回 0 成功, 非 0 失败 */
int MPU6050_DMP_Init(void);

/* 读取 DMP 欧拉角 (度), 返回 0 成功, 非 0 表示数据尚未就绪 */
int MPU6050_DMP_GetEuler(float *pitch, float *roll, float *yaw);

/* 读取原始四元数 [w, x, y, z], 返回 0 成功 */
int MPU6050_DMP_GetQuaternion(long *qw, long *qx, long *qy, long *qz);

#endif /* _MPU6050_DMP_H_ */
