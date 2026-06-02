#include "TX-16A.h"
#include "main.h"
extern UART_HandleTypeDef huart1;

void Servo_Write(uint8_t id, uint16_t position, uint16_t time)
{
while(huart1.gState != HAL_UART_STATE_READY);       //等待前一次发送完毕，再发送新的数据包

static uint8_t packet[10];

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

    HAL_UART_Transmit_DMA(&huart1, packet, sizeof(packet));   // 通过 USART1 的 DMA 发送数据包
}

void Servo_Stop(uint8_t id)
{
while(huart1.gState != HAL_UART_STATE_READY);       //等待前一次发送完毕，再发送新的数据包

static uint8_t packet[6];
    
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

    HAL_UART_Transmit_DMA(&huart1, packet, sizeof(packet));   // 通过 USART1 的 DMA 发送数据包
}
