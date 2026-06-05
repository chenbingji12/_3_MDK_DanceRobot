#include "LX-16A.h"
#include "main.h"
#include "string.h"

extern UART_HandleTypeDef huart1;

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

    if(Fifo_Write(&fifo,packet))
    {
        if(huart1.gState == HAL_UART_STATE_READY)
        {
            if(Fifo_Read(&fifo,fifo_packet))
    HAL_UART_Transmit_DMA(&huart1, fifo_packet, 10);   // 通过 USART1 的 DMA 发送数据包
        }
    }
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

fifo_t fifo = {0};               // 全局 FIFO
uint8_t fifo_packet[10]={0};        //fifo缓冲区

static uint8_t Fifo_Is_Full(fifo_t *fifo)
{
    if(((fifo->head+1)%TX_FIFO_SIZE) == ((fifo->tail)%TX_FIFO_SIZE))
    {
        return 1;
    }
    else {
    return 0;
    }
}

static uint8_t Fifo_Is_Empty(fifo_t *fifo)
{
    if((fifo->head % TX_FIFO_SIZE) == (fifo->tail % TX_FIFO_SIZE))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t Fifo_Write(fifo_t *fifo, uint8_t fifo_packet[10])
{
    if(Fifo_Is_Full(fifo))
    {
        return 0;
    }
    else
    {
        memcpy(fifo->buf[fifo->head],fifo_packet,10);
        fifo->head = (fifo->head+1)%TX_FIFO_SIZE;
        return 1;
    }
}

uint8_t Fifo_Read(fifo_t *fifo, uint8_t fifo_packet[10])
{
    if(Fifo_Is_Empty(fifo))
    {
        return 0;
    }
    else 
    {
    memcpy(fifo_packet,fifo->buf[fifo->tail],10);
    fifo->tail = (fifo->tail+1)%TX_FIFO_SIZE;
        return 1;
    }
}
