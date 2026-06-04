#ifndef __SINGLE_ACTION_H
#define __SINGLE_ACTION_H
#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "LX-16A.h"

typedef struct {
    char name[30];      //动作名称
    const uint16_t (*data)[3];   //动作数据指针，指向一个二维数组，每行包含舵机编号、目标角度和执行时间
    uint8_t count;      //动作数据行数
    uint8_t is_circular;  //是否为循环动作标志，1表示循环动作，0表示单次动作
} Action;

void Take_Action(const uint16_t a[][3], uint8_t count);

void Single_Action(char name[30]);

void Reset_Whole(void);
void Left_Step_Forward(void);
void Right_Step_Forward(void);
void Left_Step_Backward(void);
void Right_Step_Backward(void);
void Left_MoveAside(void);
void Right_MoveAside(void);

#endif
