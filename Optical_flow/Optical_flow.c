/**
 * @file    Optical_flow.c
 * @brief   光流模块实现文件
 */
#include "Optical_flow.h"

/**
 * @brief   初始化模块
 */
void OpticalFlow_Init(void)
{
    if(HAL_I2C_IsDeviceReady(&hi2c1, 0x31<<1,3,100) == HAL_OK)
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] is ready\n");
    }
    else
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] init failed\n");
    }
}

/**
 * @brief   获取光流数据
 * @retval  无
 */
static OpticalFlow_Original_Data_t* OpticalFlow_GetData(void)
{
    static OpticalFlow_Original_Data_t* original_data=NULL;
    uint8_t buffer[13];
    if(HAL_I2C_Mem_Read(&hi2c1, 0x31<<1, 0x00, I2C_MEMADD_SIZE_8BIT, buffer, 13, 100) == HAL_OK)
    {
        original_data->distance[0] = buffer[1];
        original_data->distance[1] = buffer[2];
        original_data->TOF_quality = buffer[3];
        original_data->flow_x_integral[0] = buffer[5];
        original_data->flow_x_integral[1] = buffer[6];
        original_data->flow_y_integral[0] = buffer[7];
        original_data->flow_y_integral[1] = buffer[8];
        original_data->integration_timespan[0] = buffer[9];
        original_data->integration_timespan[1] = buffer[10];
        original_data->valid = buffer[11];

        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] read success\n");
    }
    else
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] read data failed\n");
    }
    return original_data;
}

/**
 * @brief   处理原始光流数据，与IMU，TOF融合，供外部使用
 * @retval  处理好的光流数据指针
 */
OpticalFlow_Data_t* OpticalFlow_ProcessData(void)
{
    static OpticalFlow_Data_t processed_data;
    OpticalFlow_Original_Data_t* original_data = OpticalFlow_GetData();

    if (original_data != NULL)
    {
        processed_data.distance_x = (float)(original_data->flow_x_integral[0] + original_data->flow_x_integral[1]) / 10000.0f * original_data->distance[0];
        processed_data.distance_y = (float)(original_data->flow_y_integral[0] + original_data->flow_y_integral[1]) / 10000.0f * original_data->distance[1];
    }

    return &processed_data;
}
