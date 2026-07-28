/**
  * @file    LX-16A.c
  * @brief   LX-16A舵机控制函数实现
  */

#include "LX-16A.h"
#include "main.h"
#include "string.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;
extern uint8_t uart1_rx_buf;

/**
  * @brief  向舵机写入位置和时间参数
  * @param  id: 舵机ID
  * @param  position: 目标位置
  * @param  time: 执行时间
  * @retval 无
  */
void Servo_Write(uint8_t id, uint16_t position, uint16_t time)
{
    ServoWritePacket pkt = {
        .header   = {0x55, 0x55},
        .id       = id,
        .length   = 0x07,
        .cmd      = 0x01,
        .position = position,       // 编译器自动处理小端字节序
        .time     = time,
    };
    /* 计算校验和：id + length + cmd + position_low + position_high + time_low + time_high */
    uint8_t *p = (uint8_t *)&pkt;
    uint8_t sum = 0;
    for (int i = 2; i <= 8; i++) {
        sum += p[i];
    }
    pkt.checksum = ~sum;

    if (Fifo_Write(&fifo, (uint8_t *)&pkt, sizeof(pkt)) == 1) {
        if (huart1.gState == HAL_UART_STATE_READY) {
            uint8_t len;
            if (Fifo_Read(&fifo, fifo_packet, &len) == 1) {
                if (huart1.RxState != HAL_UART_STATE_READY) {
                    HAL_UART_AbortReceive(&huart1);
                }
                HAL_HalfDuplex_EnableTransmitter(&huart1);
                HAL_UART_Transmit_DMA(&huart1, fifo_packet, len);
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
    ServoStopOrReadPacket pkt = {
        .header   = {0x55, 0x55},
        .id       = id,
        .length   = 0x03,
        .cmd      = 0x0C,
    };
    uint8_t *p = (uint8_t *)&pkt;
    uint8_t sum = 0;
    for (int i = 2; i <= 4; i++) {
        sum += p[i];
    }
    pkt.checksum = ~sum;

    if(Fifo_Write(&fifo, (uint8_t *)&pkt, sizeof(pkt))==1)
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
uint16_t Servo_ReadPos(uint8_t id)
{
    ServoStopOrReadPacket pkt = {
        .header   = {0x55, 0x55},
        .id       = id,
        .length   = 0x03,
        .cmd      = 0x1C,
    };
    uint8_t *p = (uint8_t *)&pkt;
    uint8_t sum = 0;
    for (int i = 2; i <= 4; i++) {
        sum += p[i];
    }
    pkt.checksum = ~sum;

    while (huart1.gState != HAL_UART_STATE_READY) {}//等待 FIFO 排空 + 当前 DMA 发送完成

    // 半双工：发送前中止接收，切换到发送模式
    if (huart1.RxState != HAL_UART_STATE_READY)
    {
        HAL_UART_AbortReceive(&huart1);
    }
    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, (uint8_t *)&pkt, sizeof(pkt),50);// 通过 USART1 阻塞发送数据包
    HAL_HalfDuplex_EnableReceiver(&huart1);
    ServoReceivePacket rec;
    HAL_UART_Receive(&huart1, (uint8_t *)&rec, sizeof(rec),50);// 通过 USART1 阻塞接收数据包
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)uart1_rx_buf, sizeof(uart1_rx_buf));//恢复 DMA 接收
    return rec.position;
}
