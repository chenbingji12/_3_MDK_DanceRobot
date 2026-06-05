#ifndef __TASK_H
#define __TASK_H
#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "LX-16A.h"
#include "Single_action.h"

#define ACTION_NUM (sizeof(move_actions) / sizeof(move_actions[0]))     //定义计算出的动作个数

/*表驱动的时间触发合作式调度器*/
// 1. 定义任务控制块 (TCB) 结构体
typedef struct {
    void (*task_func)(void); // 函数指针：指向具体要执行的任务函数
    uint32_t interval_ms;    // 任务的期望执行周期 (单位：毫秒)
    uint32_t last_run_time;  // 记录该任务上一次执行时的系统时间戳
    uint8_t  is_active;      // 任务使能开关 (1:运行, 0:挂起休眠)
} TaskDef;

extern TaskDef task_table[];   // 任务表，存放所有任务的控制块
extern const uint8_t task_count;

typedef struct {
   volatile uint8_t *move_flag;          //动作指令
    void (*task_func_1)(void);   //动作函数1指针
    void (*task_func_2)(void);   //动作函数2指针
    uint32_t wait_time_ms;    // 任务的等待时间 (单位：毫秒)
    uint8_t count;              // 任务执行次数
} MoveAction;

extern MoveAction move_actions[];   // 动作表，存放所有动作的控制块

void Move_Action(void);    //移动任务函数

#endif
