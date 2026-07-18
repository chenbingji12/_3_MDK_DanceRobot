#ifndef __SINGLE_ACTION_H
#define __SINGLE_ACTION_H
#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "LX-16A.h"
#include "stdio.h"

#define ACTION_COUNT (sizeof(action)/sizeof(action[0]))     //定义计算出的任务个数

typedef struct {
    char name[30];      //动作名称
    uint8_t is_prefix;  //是否完全匹配，0为完全匹配，1为前缀匹配
    const uint16_t (*data)[3];   //动作数据指针，指向一个二维数组，每行包含舵机编号、目标角度和执行时间
    uint8_t count;      //动作数据行数
    uint8_t is_circular;  //是否为循环动作标志，1表示循环动作，0表示单次动作
    void (*handler)(char *param);    //指针函数，单个指令
} Action;

void Slider(char *param);
void ReadAllPos(char *param);

void Take_Action(const uint16_t a[][3], uint8_t count);

void Single_Action(char *name);

void Reset_Whole(void);
void Left_Step_Forward(void);
void Right_Step_Forward(void);
void Left_Step_Backward(void);
void Right_Step_Backward(void);
void Left_MoveAside(void);
void Right_MoveAside(void);

#endif
