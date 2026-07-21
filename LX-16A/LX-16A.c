/**
  * @file    LX-16A.c
  * @brief   LX-16A舵机控制函数实现
  */

#include "LX-16A.h"
#include "main.h"
#include "string.h"

extern UART_HandleTypeDef huart1;

/**
  * @brief  向舵机写入位置和时间参数
  * @param  id: 舵机ID
  * @param  position: 目标位置
  * @param  time: 执行时间
  * @retval 无
  */
void Servo_Write(uint8_t id, uint16_t position, uint16_t time)
{
uint8_t packet[10];

packet[0] = 0x55;   // 包头1
packet[1] = 0x55;   // 包头2

packet[2] = id;    // 舵机ID

packet[3] = 0x07;   // 长度

packet[4] = 0x01;   // 指令序号

/*拆解位置参数*/
packet[5] = position & 0xFF;     // 低8位
packet[6] = (position >> 8) & 0xFF;  // 高8位

/*拆解时间参数*/
packet[7] = time & 0xFF;         // 低8位
packet[8] = (time >> 8) & 0xFF;  // 高8位

 /*计算校验和 (对packet[2]到packet[8]求和后取反)*/
    uint16_t sum = 0;
    for (int i = 2; i <= 8; i++) {
        sum += packet[i];
    }
    packet[9] = ~(sum & 0xFF);

    if(Fifo_Write(&fifo, packet, 10)==1)
    {
        if(huart1.gState == HAL_UART_STATE_READY)
        {
            uint8_t len;
            if(Fifo_Read(&fifo, fifo_packet, &len)==1)
            {
                // 半双工：发送前中止接收，切换到发送模式
                if (huart1.RxState != HAL_UART_STATE_READY)
                {
                    HAL_UART_AbortReceive(&huart1);
                }
                HAL_HalfDuplex_EnableTransmitter(&huart1);
                HAL_UART_Transmit_DMA(&huart1, fifo_packet, len);   // 通过 USART1 的 DMA 发送数据包
            }
        }
    }
}

/**
  * @brief  停止舵机运动
  * @param  id: 舵机ID
  * @retval 无
  */
void Servo_Stop(uint8_t id)
{
//while(huart1.gState != HAL_UART_STATE_READY);       //等待前一次发送完毕，再发送新的数据包

uint8_t packet[6];
    
packet[0] = 0x55;   // 包头1
packet[1] = 0x55;   // 包头2

packet[2] = id;    // 舵机ID

packet[3] = 0x03;   // 长度

packet[4] = 0x0c;   // 指令序号

/*计算校验和 (对packet[2]到packet[4]求和后取反)*/
    uint16_t sum = 0;
    for (int i = 2; i <= 4; i++) {
        sum += packet[i];
    }
    packet[5] = ~(sum & 0xFF);

    if(Fifo_Write(&fifo, packet, 6)==1)
    {
        if(huart1.gState == HAL_UART_STATE_READY)
        {
            uint8_t len;
            if(Fifo_Read(&fifo, fifo_packet, &len)==1)
            {
                // 半双工：发送前中止接收，切换到发送模式
                if (huart1.RxState != HAL_UART_STATE_READY)
                {
                    HAL_UART_AbortReceive(&huart1);
                }
                HAL_HalfDuplex_EnableTransmitter(&huart1);
                HAL_UART_Transmit_DMA(&huart1, fifo_packet, len);   // 通过 USART1 的 DMA 发送数据包
            }
        }
    }
}

/**
  * @brief  读取舵机位置
  * @param  id: 舵机ID
  * @retval 无
  */
void Servo_ReadPos(uint8_t id)
{
    uint8_t packet[6];

    packet[0]=0x55;
    packet[1]=0x55;

    packet[2]=id;

    packet[3]=0x03;

    packet[4]=0x1C;

    packet[5]=~(id + 0x03 + 0x1C) & 0xFF;

    // 半双工：发送前中止接收，切换到发送模式
    if (huart1.RxState != HAL_UART_STATE_READY)
    {
        HAL_UART_AbortReceive(&huart1);
    }
    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit_DMA(&huart1, packet, 6);
}
