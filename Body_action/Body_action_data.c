/**
 * @file    Body_action_data.c
 * @brief   体动作数据定义
 */

#include "Body_action.h"

/**
 * @brief   机械臂第一个动作数据：1，3关节摇摆，三维数组，第一维为动作帧数，第二维为舵机个数，第三维为舵机编号、目标角度和执行时间
 */
const uint16_t arm_action_data_1[][4][3]={
  {{17,350,1000},{18,500,1000},{19,610,1000},{20,500,1000}},
  {{17,750,1000},{18,500,1000},{19,305,1000},{20,500,1000}},
};
const uint8_t arm_action_count_1 = sizeof(arm_action_data_1) / sizeof(arm_action_data_1[0]); // 动作帧数

/**
 * @brief   机械臂第二个动作数据
 */
const uint16_t arm_action_data_2[][4][3]={
  {{17,350,1000},{18,500,1000},{19,610,1000},{20,500,1000}},
  {{17,750,1000},{18,500,1000},{19,305,1000},{20,500,1000}},
};
const uint8_t arm_action_count_2 = sizeof(arm_action_data_2) / sizeof(arm_action_data_2[0]); // 动作帧数
