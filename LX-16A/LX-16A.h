#ifndef __LX_16A_H
#define __LX_16A_H

#include "stm32f4xx.h"                  // Device header

#define TX_FIFO_SIZE (19)       //定义FIFO环形缓冲区大小

typedef struct{
    uint8_t buf[TX_FIFO_SIZE][10];      //数组环形缓冲区
    volatile uint8_t head;      //写指针
    volatile uint8_t tail;      //读指针
}fifo_t;

extern fifo_t fifo;

extern uint8_t fifo_packet[10];

void Servo_Write(uint8_t id, uint16_t position, uint16_t time);
void Servo_Stop(uint8_t id);
uint8_t Fifo_Write(fifo_t *fifo, uint8_t fifo_packet[10]);
uint8_t Fifo_Read(fifo_t *fifo, uint8_t fifo_packet[10]);

#endif

