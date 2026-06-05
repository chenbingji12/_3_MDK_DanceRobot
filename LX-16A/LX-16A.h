#ifndef __LX_16A_H
#define __LX_16A_H

#include "stm32f4xx.h"                  // Device header

#define TX_FIFO_SIZE (20)

typedef struct{
    uint8_t buf[TX_FIFO_SIZE][10];
    volatile uint8_t head;
    volatile uint8_t tail;
}fifo_t;

extern fifo_t fifo;

extern uint8_t fifo_packet[10];

void Servo_Write(uint8_t id, uint16_t position, uint16_t time);
void Servo_Stop(uint8_t id);
uint8_t Fifo_Write(fifo_t *fifo, uint8_t fifo_packet[10]);
uint8_t Fifo_Read(fifo_t *fifo, uint8_t fifo_packet[10]);

#endif

