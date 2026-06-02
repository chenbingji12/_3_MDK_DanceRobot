#ifndef __TX_16A_H
#define __TX_16A_H

#include "stm32f4xx.h"                  // Device header

void Servo_Write(uint8_t id, uint16_t position, uint16_t time);
void Servo_Stop(uint8_t id);

#endif

