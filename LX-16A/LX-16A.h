/**
  * @file    LX-16A.h
  * @brief   LX-16A舵机控制函数声明
  */

#ifndef __LX_16A_H
#define __LX_16A_H

#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "FIFO.h"

/**
  * @brief  舵机写入的结构体,10 字节：舵机移动指令 (CMD=0x01)
  */
typedef __packed struct {
    uint8_t  header[2];    // 0x55 0x55
    uint8_t  id;           // 舵机 ID
    uint8_t  length;       // 0x07
    uint8_t  cmd;          // 0x01
    uint16_t position;     // 目标位置 (小端)
    uint16_t time;         // 执行时间 (小端)
    uint8_t  checksum;     // 校验和
} ServoWritePacket;

/**
 * @brief   6 字节：停止/读取共用指令 (CMD=0x0C/0x1C)
  */
typedef __packed struct {
    uint8_t  header[2];    // 0x55 0x55
    uint8_t  id;           // 舵机 ID
    uint8_t  length;       // 0x03
    uint8_t  cmd;          // 0x0C/0x1C
    uint8_t  checksum;     // 校验和
} ServoStopOrReadPacket;

/**
 * @brief   接收舵机反馈的角度
  */
typedef __packed struct {
    uint8_t  header[2];    // 0x55 0x55
    uint8_t  id;           // 舵机 ID
    uint8_t  length;       // 0x05
    uint8_t  cmd;          // 0x1C
    uint16_t position;     // 舵机位置 (小端)
    uint8_t  checksum;     // 校验和
} ServoReceivePacket;

void Servo_Write(uint8_t id, uint16_t position, uint16_t time);
void Servo_Stop(uint8_t id);
uint16_t Servo_ReadPos(uint8_t id);

#endif

