/**
 * @file    WS2812.c
 * @brief   WS2812B灯带PWM+DMA驱动实现
 * @note    基于TIM3_CH3(PB0)环形DMA, 整帧循环发送
 *          硬件初始化由CubeMX管理(MX_TIM3_Init),
 *          DMA CIRCULAR模式 → 永不停止, TC中断重建缓冲
 */

#include "WS2812.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/*============================================================================
 * 外部句柄
 *============================================================================*/

extern TIM_HandleTypeDef htim3;

/*============================================================================
 * 内部状态
 *============================================================================*/

static WS2812_Color_t led_colors[WS2812_LED_COUNT];

/* DMA缓冲区: 60LED数据 + 3LED全零 = 1512个uint16_t */
static uint16_t dma_ccr_buf[WS2812_DMA_BUF_SIZE]
    __attribute__((aligned(4)));

/* 刷新请求标志: 1=TC中断中需要重建缓冲 */
static volatile uint8_t refresh_pending = 0U;

/*============================================================================
 * 私有函数
 *============================================================================*/

/**
 * @brief  将单个LED编码为24个CCR值 (GRB序)
 */
static void WS2812_EncodeLED(uint16_t idx, uint16_t *dst)
{
    WS2812_Color_t c = led_colors[idx];
    uint8_t  grb[3] = { c.g, c.r, c.b };
    uint16_t pos     = 0U;

    for (uint8_t byte = 0U; byte < 3U; byte++) {
        for (uint8_t mask = 0x80U; mask != 0U; mask >>= 1) {
            dst[pos] = (grb[byte] & mask) ? WS2812_CCR_1 : WS2812_CCR_0;
            pos++;
        }
    }
}

/**
 * @brief  重建DMA缓冲: 60LED数据 + 末尾3LED全零
 * @note   在TC中断中调用, CPU速度远快于DMA, 不出竞争
 */
static void WS2812_RebuildBuffer(void)
{
    uint16_t pos = 0U;

    for (uint16_t i = 0U; i < WS2812_LED_COUNT; i++) {
        WS2812_EncodeLED(i, &dma_ccr_buf[pos]);
        pos += 24U;
    }

    for ( ; pos < WS2812_DMA_BUF_SIZE; pos++) {
        dma_ccr_buf[pos] = 0U;
    }
}

/*============================================================================
 * 公开API
 *============================================================================*/

void WS2812_Init(void)
{
    (void)memset(led_colors,  0, sizeof(led_colors));
    (void)memset(dma_ccr_buf, 0, sizeof(dma_ccr_buf));

    WS2812_RebuildBuffer();
    refresh_pending = 0U;

    /* 启动DMA后永不停止 */
    if (HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3,
                              (uint32_t *)dma_ccr_buf,
                              WS2812_DMA_BUF_SIZE) != HAL_OK) {
        Error_Handler();
    }
}

void WS2812_SetLED(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < WS2812_LED_COUNT) {
        led_colors[idx].r = r;
        led_colors[idx].g = g;
        led_colors[idx].b = b;
    }
}

/**
 * @brief  填充全部LED → 仅更新颜色数组, 设置刷新标志
 * @note   缓冲重建推迟到TC中断, DMA零停机
 */
void WS2812_Fill(uint8_t r, uint8_t g, uint8_t b)
{
    WS2812_Color_t c = {r, g, b};
    for (uint16_t i = 0U; i < WS2812_LED_COUNT; i++) {
        led_colors[i] = c;
    }
    refresh_pending = 1U;
}

void WS2812_Clear(void)
{
    WS2812_Fill(0U, 0U, 0U);
}

uint16_t WS2812_GetLEDCount(void)
{
    return WS2812_LED_COUNT;
}

/*============================================================================
 * HAL DMA全传输完成回调 → 在此重建缓冲, 零停机刷新
 *============================================================================*/

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        if (refresh_pending) {
            WS2812_RebuildBuffer();
            refresh_pending = 0U;
        }
    }
}

/*============================================================================
 * 命令处理函数
 *============================================================================*/

void WS2812_TestCmd(char *param)
{
    flag.ws2812_change = 1;   //WS2812B颜色渐变任务激活
}

void WS2812_FillCmd(char *param)
{
    int r = 0, g = 0, b = 0;
    if (sscanf(param, "%d %d %d", &r, &g, &b) == 3) {
        WS2812_Fill((uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
}

void WS2812_OffCmd(char *param)
{
    (void)param;
    flag.ws2812_change = 0;   //WS2812B颜色渐变任务挂起
    WS2812_Clear();
}
