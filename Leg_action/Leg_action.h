/**
 * @file    Leg_action.h
 * @brief   腿部动作控制函数声明
 */

#ifndef __LEG_ACTION_H
#define __LEG_ACTION_H

#include "main.h"

extern const uint16_t Init_To_Half_Stand_Data[][16][3]; // 从初始状态到半起立动作数据
extern const uint8_t Init_To_Half_Stand_Count; // 从初始状态到半起立动作的帧数

extern const uint16_t Half_Stand_To_Full_Stand_Data[][16][3]; // 从半起立到完全起立动作数据
extern const uint8_t Half_Stand_To_Full_Stand_Count; // 从半起立到完全起立动作的帧数

extern const uint16_t Walk_Forward_Data[][16][3]; // 行走前进动作数据
extern const uint8_t Walk_Forward_Count; // 行走前进动作的帧数

/**
 *@brief   腿部动作调度结构体
 */
typedef struct{
    void (*action_func)(void);   //动作函数指针
    uint32_t last_run_time;  //记录该动作上一次执行时的系统时间戳
    uint16_t *interval_ms;    //动作执行间隔时间 (单位：毫秒)
    uint8_t  *is_active;      //动作使能开关 (1:运行, 0:挂起休眠)
}LegAction;

extern LegAction leg_action_table[];   // 腿部动作表，存放所有动作的控制块

extern const uint8_t leg_action_count; // 腿部动作表中动作的数量

void Init_To_Half_Stand(void);    // 从初始状态到半起立动作的执行函数
void Half_Stand_To_Init(void);    // 从半起立到初始状态动作的执行函数
void Half_Stand_To_Full_Stand(void);    // 从半起立到完全起立动作的执行函数
void Full_Stand_To_Half_Stand(void);    // 从完全起立到半起立动作的执行函数
void Walk_Forward(void);    // 前进动作的执行函数

void Leg_Action_Process(void);    // 腿部动作调度函数

#endif
