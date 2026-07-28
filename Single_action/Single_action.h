/**
  * @file    Single_action.h
  * @brief   单一动作控制函数声明
  */

#ifndef __SINGLE_ACTION_H
#define __SINGLE_ACTION_H

#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "LX-16A.h"
#include "stdio.h"

/**
  * @brief  定义计算出的任务个数
  */
#define ACTION_COUNT (sizeof(action)/sizeof(action[0]))

/**
  * @brief  定义动作结构体
  */
typedef struct {
    char name[30];      //动作名称
    uint8_t is_prefix;  //是否完全匹配，0为完全匹配，1为前缀匹配
    const uint16_t (*data)[3];   //动作数据指针，指向一个二维数组，每行包含舵机编号、目标角度和执行时间
    uint8_t count;      //动作数据行数
    uint8_t is_circular;  //是否为循环动作标志，1表示循环动作，0表示单次动作
    void (*handler)(char *param);    //指针函数，单个指令
} Action;

/**
  * @brief  定义单个动作函数
  */
void Slider(char *param);
void ReadAllPos(char *param);
void Set_Servo_Pos(char *param);
void Read_Servo_Pos(char *param);

/**
  * @brief  执行动作
  * @param a 动作数据指针
  * @param count 动作数据行数
  * @retval 无
  */
void Take_Action(const uint16_t a[][3], uint8_t count);

/**
  * @brief  执行单个动作
  * @param name 动作名称
  * @retval 无
  */
void Single_Action(char *name);

/**
  * @brief  复位全身动作薄函数
  * @retval 无
  */
void Reset_Whole(void);
void Left_Step_Forward(void);
void Right_Step_Forward(void);
void Left_Step_Backward(void);
void Right_Step_Backward(void);
void Left_MoveAside(void);
void Right_MoveAside(void);

#endif
