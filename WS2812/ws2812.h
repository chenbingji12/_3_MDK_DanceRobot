/**
 * @file    ws2812.h
 * @brief   WS2812B灯带PWM+DMA驱动头文件
 * @note    基于TIM3_CH3(PB0)环形DMA+HT/TC双缓冲
 *          硬件初始化由CubeMX管理, 本模块仅负责数据编码与刷新
 */

#ifndef __WS2812_H
#define __WS2812_H

#include <stdint.h>
#include "main.h"

/*============================================================================
 * 灯带参数
 *============================================================================*/

/* LED数量 */
#define WS2812_LED_COUNT        250U

/* 每次DMA中断(半缓冲)处理的LED数量 */
#define WS2812_LEDS_PER_DMA_IRQ 4U

/* DMA缓冲区大小: 双半缓冲 × 每半4颗LED × 每颗24bit */
#define WS2812_DMA_BUF_SIZE     (2U * WS2812_LEDS_PER_DMA_IRQ * 24U)

/* Reset低电平脉冲数(>50us, 800kHz下50脉冲=62.5us) */
#define WS2812_RESET_PULSES     50U

/* PWM CCR值: 0码和1码 (ARR=124, 800kHz)
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
 *         启动后自动连续刷新LED, 通过WS2812_Process()维持数据流
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
 * @brief  填充全部LED为同一颜色
 * @param  r     红色分量 0-255
 * @param  g     绿色分量 0-255
 * @param  b     蓝色分量 0-255
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

/**
 * @brief  主循环处理函数, 维持DMA数据流
 * @note   非阻塞, 应在主循环每次迭代中调用
 *         检查HT/TC标志, 填充下一批LED数据到DMA缓冲
 */
void WS2812_Process(void);

/*============================================================================
 * 命令处理函数 (供Single_action命令表使用)
 * 签名匹配 void (*handler)(char *param)
 *============================================================================*/

void WS2812_TestCmd(char *param);
void WS2812_FillCmd(char *param);
void WS2812_OffCmd(char *param);

#endif /* __WS2812_H */
