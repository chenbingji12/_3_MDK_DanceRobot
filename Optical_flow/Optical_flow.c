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
    if(HAL_I2C_IsDeviceReady(&hi2c1, 0x31<<1,3,500) == HAL_OK)
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] is ready\n");
    }
    else
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] init failed\n");
        while(1);//初始化失败，死循环，等看门狗复位重试
    }
}

/**
 * @brief   获取光流数据
 * @retval  无
 */
static OpticalFlow_Original_Data_t* OpticalFlow_GetData(volatile uint8_t* flow_data_update)
{
    static OpticalFlow_Original_Data_t original_data;
    uint8_t buffer[13];
    if(HAL_I2C_Mem_Read(&hi2c1, 0x31<<1, 0x00, I2C_MEMADD_SIZE_8BIT, buffer, 13, 50) == HAL_OK)
    {
        if(buffer[11]==0)
        {
            return NULL;
        }
        original_data.distance[0] = buffer[1];
        original_data.distance[1] = buffer[2];
        original_data.TOF_quality = buffer[3];
        original_data.flow_x_integral[0] = buffer[5];
        original_data.flow_x_integral[1] = buffer[6];
        original_data.flow_y_integral[0] = buffer[7];
        original_data.flow_y_integral[1] = buffer[8];
        original_data.integration_timespan[0] = buffer[9];
        original_data.integration_timespan[1] = buffer[10];
        original_data.valid = buffer[11];
        original_data.version = buffer[12];

        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] read success\n");

        *flow_data_update=1;//更新光流数据标志位

    }
    else
    {
        (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[OpticalFlow] read data failed\n");
        return NULL;
    }
    return &original_data;
}

/**
 * @brief   处理原始光流数据，与IMU，TOF融合，供外部使用
 * @retval  处理好的光流数据指针
 */
OpticalFlow_Data_t* OpticalFlow_ProcessData(volatile uint8_t* flow_data_update)
{
    static OpticalFlow_Data_t processed_data;
    OpticalFlow_Original_Data_t* original_data = OpticalFlow_GetData(flow_data_update);

    if(*flow_data_update==0||original_data==NULL)
    {
        return NULL;
    }

    *flow_data_update=0;

    //距离地面距离单位：mm
    uint16_t distance_to_ground=(uint16_t)(original_data->distance[0]|(original_data->distance[1]<<8));

    //未融合的位移数据，单位：mm
    int16_t flow_x=(int16_t)(original_data->flow_x_integral[0]|(original_data->flow_x_integral[1]<<8));
    int16_t flow_y=(int16_t)(original_data->flow_y_integral[0]|(original_data->flow_y_integral[1]<<8));

    //计算后的累计位移，单位：mm
    float flow_x_integral=(float)(((float)flow_x)/10000.0f*((float)distance_to_ground));
    float flow_y_integral=(float)(((float)flow_y)/10000.0f*((float)distance_to_ground));

    processed_data.distance_x=flow_x_integral;
    processed_data.distance_y=flow_y_integral;

    ////上一次发送光流数据到本次发送光流数据的累计时间，微秒
    int16_t integration_timespan=(int16_t)(original_data->integration_timespan[0]|(original_data->integration_timespan[1]<<8));

    return &processed_data;
}
