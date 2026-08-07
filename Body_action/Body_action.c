/**
  * @file    Body_action.c
  * @brief   体动作函数定义
  */

#include "Body_action.h"
#include <stdint.h>

uint8_t arm_action_1_active = 0; // 机械臂动作一是否激活 (1:激活, 0:未激活)
uint8_t arm_action_1_frame_index = 0; // 当前机械臂动作的帧索引
uint16_t Arm_Action_1_Step_Time = 0; // 机械臂第一个动作每帧的执行时间 (毫秒)

uint8_t arm_action_2_active = 0; // 机械臂动作二是否激活 (1:激活, 0:未激活)
uint8_t arm_action_2_frame_index = 0; // 当前机械臂动作的帧索引
uint16_t Arm_Action_2_Step_Time = 0; // 机械臂第二个动作每帧的执行时间 (毫秒)

/**
 * @brief   计算完成当前帧所需的时间
 * @param   array: 三维数组，包含动作数据
 * @param   frame_index: 当前帧索引
 * @return  所需时间 (毫秒)
 */
static uint16_t Max_Frame_Time(const uint16_t array[][4][3], uint8_t frame_index)
{
    uint16_t max_time = 0;
    for (uint8_t i = 0; i < 4; i++)
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
static void Send_Data_to_Servo(const uint16_t array[][4][3], uint8_t index)
{
    for (uint8_t i = 0; i < 4; i++) {
        Servo_Write((uint8_t)array[index][i][0],array[index][i][1],array[index][i][2]);
    }
}

/**
 * @brief   执行机械臂动作表
 */
ArmAction arm_action_table[] = {
    {Arm_Action_1, 0, &Arm_Action_1_Step_Time, &arm_action_1_active},
    {Arm_Action_2, 0, &Arm_Action_2_Step_Time, &arm_action_2_active}
};

const uint8_t arm_action_count = sizeof(arm_action_table) / sizeof(arm_action_table[0]); // 机械臂动作表中动作的数量

/**
 * @brief   机械臂第一个动作的执行函数,单次
 */
void Arm_Action_1(char *param)
{
    arm_action_1_active = 1; // 设置机械臂动作一标志为运行状态
    Send_Data_to_Servo(arm_action_data_1, arm_action_1_frame_index); // 发送当前帧的数据给舵机
    Arm_Action_1_Step_Time = Max_Frame_Time(arm_action_data_1, arm_action_1_frame_index); // 计算当前帧所需的时间
    arm_action_1_frame_index = (arm_action_1_frame_index + 1) % arm_action_count_1; // 更新帧索引
    if (arm_action_1_frame_index == 0) {
        arm_action_1_active = 0; // 动作完成后，设置标志为未激活
    }
}

/**
 * @brief   机械臂第二个动作的执行函数，单次
 */
void Arm_Action_2(char *param)
{
    arm_action_2_active = 1; // 设置机械臂动作二标志为运行状态
    Send_Data_to_Servo(arm_action_data_2, arm_action_2_frame_index); // 发送当前帧的数据给舵机
    Arm_Action_2_Step_Time = Max_Frame_Time(arm_action_data_2, arm_action_2_frame_index); // 计算当前帧所需的时间
    arm_action_2_frame_index = (arm_action_2_frame_index + 1) % arm_action_count_2; // 更新帧索引
    if (arm_action_2_frame_index == 0) {
        arm_action_2_active = 0; // 动作完成后，设置标志为未激活
    }
}

/**
 * @brief   任务函数，用于执行机械臂动作表中的动作
 */
void Arm_Action_Process(void)
{
    for (uint8_t i = 0; i < arm_action_count; i++)
    {
        if (*(arm_action_table[i].is_active)) // 检查动作是否使能
        {
            uint32_t current_time = HAL_GetTick(); // 获取当前系统时间戳
            if (current_time - arm_action_table[i].last_run_time >= *(arm_action_table[i].interval_ms)) // 检查是否到达执行间隔
            {
                arm_action_table[i].action_func(NULL); // 执行动作函数
                arm_action_table[i].last_run_time = current_time; // 更新上一次执行时间戳
            }
        }
    }
}
