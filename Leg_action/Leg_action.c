/**
 * @file    Leg_action.c
 * @brief   腿部动作控制函数实现
 */

#include "Leg_action.h"

uint8_t walk_forward_frame_index = 0; // 当前行走前进动作的帧索引
uint16_t Walk_Forward_Step_Time = 1000; // 行走前进动作每帧的执行时间 (毫秒)

/**
 * @brief   计算完成当前帧所需的时间
 * @param   array: 三维数组，包含动作数据
 * @param   frame_index: 当前帧索引
 * @return  所需时间 (毫秒)
 */
static uint16_t Max_Frame_Time(const uint16_t array[][15][3], uint8_t frame_index)
{
    uint16_t max_time = 0;
    for (uint8_t i = 0; i < 15; i++)
    {
        if (array[frame_index][i][2] > max_time)
        {
            max_time = array[frame_index][i][2];
        }
    }
    return max_time;
}

/**
 * @brief   将数据发送给舵机
 * @param   array: 舵机数据数组
 * @param   index: 当前帧索引
 */
static void Send_Data_to_Servo(const uint16_t array[][15][3], uint8_t index)
{
    for (uint8_t i = 0; i < 15; i++) {
        Servo_Write((uint8_t)array[index][i][0],array[index][i][1],array[index][i][2]);
    }
}

/**
 * @brief   腿部动作表
 */
LegAction leg_action_table[] = {
    {Walk_Forward, 0, &Walk_Forward_Step_Time, (uint8_t*)&flag.walk_forward},
};

const uint8_t leg_action_count = sizeof(leg_action_table) / sizeof(leg_action_table[0]); // 腿部动作表中动作的数量

/**
 * @brief   前进动作的执行函数
 */
void Walk_Forward(void)
{
Send_Data_to_Servo(Walk_Forward_Data,walk_forward_frame_index); // 发送当前帧的数据给舵机
Walk_Forward_Step_Time = Max_Frame_Time(Walk_Forward_Data, walk_forward_frame_index); // 计算当前帧所需的时间
walk_forward_frame_index = (walk_forward_frame_index + 1) % Walk_Forward_Count; // 更新帧索引
}

/**
 * @brief  任务调度
 */
void Leg_Action_Process(void)
{
    for (uint8_t i = 0; i < leg_action_count; i++)      // 遍历任务列表
    {
        if (*leg_action_table[i].is_active==1)      // 如果任务已激活
        {
            if (HAL_GetTick() - leg_action_table[i].last_run_time >= *leg_action_table[i].interval_ms)      // 如果时间到
            {
                leg_action_table[i].last_run_time = HAL_GetTick();      // 更新上次运行时间
                leg_action_table[i].action_func();      // 执行任务
            }
        }
    }
}
