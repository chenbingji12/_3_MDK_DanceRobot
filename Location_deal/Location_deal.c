///**
// * @file    Location_deal.c
// * @brief   位置处理函数定义
// */

//#include "Location_deal.h"

//IMU_Data_t *imu = IMU_GetData();//获取IMU数据指针
//float yaw = 0.0f;
//float pitch = 0.0f;
//float roll = 0.0f;

//float self_yaw = 0.0f;//软件自校准yaw角度
//float yaw_error = 0.0f;//yaw 偏差

///**
// * @brief   获取当前IMU数据
// * @retval  无
// */
//void Location_deal_GetIMUData(void)
//{
//    if (imu->updated)
//    {
//        roll = imu->roll;
//        pitch = imu->pitch;
//        yaw = imu->yaw;
//        imu->updated = 0;
//    }
//}

///**
// * @brief   软件归零（设置当前yaw为0度）
// * @retval  无
// */
//void Location_deal_ZeroYaw(void)
//{
//    self_yaw = yaw;
//    yaw_error = 0.0f;
//}
