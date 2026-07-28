/**
 * @file    Single_action_data.c
 * @brief   存放单一动作数据
 */

#include "Single_action.h"

/**
 * @brief  重置所有舵机角度为默认
 */
const uint16_t Reset_Whole_Data[][3] = {
    {1, 500, 0},  {2, 500, 0},  {3, 500, 0},  {4, 500, 0},  {5, 500, 0},
    {6, 500, 0},  {7, 500, 0},  {8, 500, 0},  {9, 500, 0},  {10, 500, 0},
    {11, 500, 0}, {12, 500, 0}, {13, 500, 0}, {14, 500, 0}, {15, 500, 0},
    {16, 500, 0}, {17, 500, 0}, {18, 500, 0}, {19, 500, 0}
};
const uint8_t Reset_Whole_Count = sizeof(Reset_Whole_Data) / sizeof(Reset_Whole_Data[0]);
