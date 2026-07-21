/**
  * @file    LX-16A.h
  * @brief   LX-16A舵机控制函数声明
  */

#ifndef __LX_16A_H
#define __LX_16A_H

#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "FIFO.h"

void Servo_Write(uint8_t id, uint16_t position, uint16_t time);
void Servo_Stop(uint8_t id);
void Servo_ReadPos(uint8_t id);

#endif

