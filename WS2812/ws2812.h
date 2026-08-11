/**
 * @file    ws2812.h
 * @brief   WS2812B灯带PWM+DMA驱动头文件
 * @note    基于TIM3_CH3(PB0)环形DMA整帧循环发送
 *          DMA永不停止, TC中断中重建缓冲, 无闪烁
 */

#ifndef __WS2812_H
#define __WS2812_H

#include <stdint.h>
#include "main.h"

/*============================================================================
 * 灯带参数 (ARR=124, 800kHz PWM)
 *============================================================================*/

/* LED数量 */
#define WS2812_LED_COUNT        60U

/* 缓冲区LED总数: 真实LED + 末尾3颗全零(帧间复位 ≈ 90μs > 50μs) */
#define WS2812_BUF_LED_COUNT    (WS2812_LED_COUNT + 3U)

/* DMA缓冲区大小: 整帧(含帧间复位) */
#define WS2812_DMA_BUF_SIZE     (WS2812_BUF_LED_COUNT * 24U)

/* PWM CCR值: 0码和1码
 * 0码 ≈ 0.4us → CCR=40 (40/125 = 32%)
 * 1码 ≈ 0.8us → CCR=80 (80/125 = 64%) */
#define WS2812_CCR_0            40U
#define WS2812_CCR_1            80U

/*============================================================================
 * 数据类型
 *============================================================================*/

/* RGB颜色结构体 */
typedef struct {
    uint8_t r;    /* 红色分量 0-255 */
    uint8_t g;    /* 绿色分量 0-255 */
    uint8_t b;    /* 蓝色分量 0-255 */
} WS2812_Color_t;

/*============================================================================
 * 公开API
 *============================================================================*/

/**
 * @brief  初始化WS2812驱动, 配置TIM3+DMA+GPIO并启动输出
 * @note   必须在MX_DMA_Init()之后调用
 *         启动后自动循环发送整帧(60LED+复位脉冲)
 */
void WS2812_Init(void);

/**
 * @brief  设置指定LED颜色
 * @param  idx   LED索引 (0 ~ WS2812_LED_COUNT-1)
 * @param  r     红色分量 0-255
 * @param  g     绿色分量 0-255
 * @param  b     蓝色分量 0-255
 */
void WS2812_SetLED(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief  填充全部LED为同一颜色 (下次TC中断生效)
 * @param  r     红色分量 0-255
 * @param  g     绿色分量 0-255
 * @param  b     蓝色分量 0-255
 * @note   仅更新颜色数组+置标志, TC中断中重建缓冲, DMA不停机
 */
void WS2812_Fill(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief  清空全部LED (熄灭)
 */
void WS2812_Clear(void);

/**
 * @brief  获取LED总数
 * @retval LED数量
 */
uint16_t WS2812_GetLEDCount(void);

/*============================================================================
 * 命令处理函数 (供Single_action命令表使用)
 * 签名匹配 void (*handler)(char *param)
 *============================================================================*/

void WS2812_TestCmd(char *param);
void WS2812_FillCmd(char *param);
void WS2812_OffCmd(char *param);

#endif /* __WS2812_H */
