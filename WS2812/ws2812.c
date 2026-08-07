/**
 * @file    WS2812.c
 * @brief   WS2812B灯带PWM+DMA驱动实现
 * @note    基于TIM3_CH3(PB0)环形DMA+HT/TC双缓冲
 *          硬件初始化由CubeMX管理(MX_TIM3_Init),
 *          本模块仅负责数据编码与双缓冲刷新
 */

#include "WS2812.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/*============================================================================
 * 外部句柄 (由CubeMX在tim.c中生成, 避免重复初始化)
 * 硬件: TIM3_CH3(PB0, AF2), DMA1_Stream7 CH5, 800kHz PWM
 *============================================================================*/

extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_tim3_ch3;

/*============================================================================
 * 内部状态 (全部static封装)
 *============================================================================*/

/* LED颜色数组 */
static WS2812_Color_t led_colors[WS2812_LED_COUNT];

/* DMA CCR缓冲区: 每个LED 24个CCR值, 双半缓冲 */
static uint16_t dma_ccr_buf[WS2812_DMA_BUF_SIZE]
    __attribute__((aligned(4)));

/* 半缓冲就绪标志 (bit0=HT前半就绪, bit1=TC后半就绪) */
static volatile uint8_t half_flag = 0U;

/* 发送游标: 下一个要编码的LED索引 */
static volatile uint16_t send_cursor = 0U;

/* Reset阶段计数 */
static uint8_t reset_count = 0U;

/* DMA运行标志 */
static uint8_t dma_running = 0U;

/*============================================================================
 * 私有函数声明
 *============================================================================*/

static void WS2812_EncodeLED(uint16_t idx, uint16_t *dst);
static void WS2812_FillHalf(uint8_t half_idx);
static void WS2812_PreFillBuffer(void);

/*============================================================================
 * 公开API实现
 *============================================================================*/

/**
 * @brief  初始化WS2812驱动
 * @note   依赖CubeMX已完成的TIM3+DMA+GPIO初始化(MX_TIM3_Init)
 *         清零缓冲 → 预填充 → 启动DMA
 */
void WS2812_Init(void)
{
    /* 清零所有缓冲区 */
    (void)memset(led_colors,  0, sizeof(led_colors));
    (void)memset(dma_ccr_buf, 0, sizeof(dma_ccr_buf));

    /* 状态变量复位 */
    half_flag   = 0U;
    send_cursor = 0U;
    reset_count = 0U;
    dma_running = 0U;

    /* 预填充DMA缓冲 */
    WS2812_PreFillBuffer();

    /* 启动TIM3_CH3 PWM DMA输出 (使用CubeMX生成的htim3句柄) */
    if (HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3,
                              (uint32_t *)dma_ccr_buf,
                              WS2812_DMA_BUF_SIZE) != HAL_OK) {
        Error_Handler();
    }
    dma_running = 1U;
}

/**
 * @brief  设置指定LED颜色
 * @param  idx   LED索引 (0 ~ WS2812_LED_COUNT-1)
 * @param  r     红色分量 0-255
 * @param  g     绿色分量 0-255
 * @param  b     蓝色分量 0-255
 */
void WS2812_SetLED(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < WS2812_LED_COUNT) {
        led_colors[idx].r = r;
        led_colors[idx].g = g;
        led_colors[idx].b = b;
    }
}

/**
 * @brief  填充全部LED为同一颜色
 * @param  r     红色分量 0-255
 * @param  g     绿色分量 0-255
 * @param  b     蓝色分量 0-255
 */
void WS2812_Fill(uint8_t r, uint8_t g, uint8_t b)
{
    WS2812_Color_t c = {r, g, b};
    for (uint16_t i = 0U; i < WS2812_LED_COUNT; i++) {
        led_colors[i] = c;
    }
}

/**
 * @brief  清空全部LED (熄灭)
 */
void WS2812_Clear(void)
{
    WS2812_Fill(0U, 0U, 0U);
}

/**
 * @brief  获取LED总数
 * @retval LED数量
 */
uint16_t WS2812_GetLEDCount(void)
{
    return WS2812_LED_COUNT;
}

/**
 * @brief  主循环处理函数, 维持DMA数据流
 * @note   非阻塞, 检查HT/TC标志并填充对应半缓冲
 */
void WS2812_Process(void)
{
    if (dma_running == 0U) {
        return;
    }

    /* 前半缓冲就绪 (HT) */
    if ((half_flag & 0x01U) != 0U) {
        __disable_irq();
        half_flag &= (uint8_t)~0x01U;
        __enable_irq();
        WS2812_FillHalf(0U);
    }

    /* 后半缓冲就绪 (TC) */
    if ((half_flag & 0x02U) != 0U) {
        __disable_irq();
        half_flag &= (uint8_t)~0x02U;
        __enable_irq();
        WS2812_FillHalf(1U);
    }
}

/*============================================================================
 * 私有函数实现
 *============================================================================*/

/**
 * @brief  将单个LED的GRB 24位编码为24个CCR值
 * @param  idx  LED索引
 * @param  dst  目标缓冲区(24个uint16_t)
 * @note   WS2812数据序: G7..G0, R7..R0, B7..B0 (GRB)
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
 * @brief  填充DMA半缓冲区(4颗LED或reset零脉冲)
 * @param  half_idx  0=前半缓冲, 1=后半缓冲
 * @note   send_cursor遍历所有LED后进入reset阶段
 *         连续填充WS2812_RESET_PULSES个零脉冲后自动重启新帧
 */
static void WS2812_FillHalf(uint8_t half_idx)
{
    uint16_t base = half_idx * WS2812_LEDS_PER_DMA_IRQ * 24U;

    for (uint8_t i = 0U; i < WS2812_LEDS_PER_DMA_IRQ; i++) {
        if (send_cursor < WS2812_LED_COUNT) {
            WS2812_EncodeLED(send_cursor, &dma_ccr_buf[base + i * 24U]);
            send_cursor++;
        } else {
            /* Reset阶段: 填零脉冲 */
            for (uint8_t j = 0U; j < 24U; j++) {
                dma_ccr_buf[base + i * 24U + j] = 0U;
            }
        }
    }

    /* 所有LED发完 → 进入reset阶段 */
    if (send_cursor >= WS2812_LED_COUNT) {
        reset_count++;
        /* 连续填满足够reset脉冲后重启新帧 */
        if (reset_count >= (WS2812_RESET_PULSES / (WS2812_LEDS_PER_DMA_IRQ * 24U) + 1U)) {
            reset_count = 0U;
            send_cursor = 0U;
        }
    }
}

/**
 * @brief  预填充整个DMA缓冲(前4颗LED)
 *         在Init启动DMA前调用, 确保首帧数据就绪
 */
static void WS2812_PreFillBuffer(void)
{
    send_cursor = 0U;
    reset_count = 0U;
    /* 填前半缓冲: LED 0-3 */
    for (uint8_t i = 0U; i < WS2812_LEDS_PER_DMA_IRQ; i++) {
        WS2812_EncodeLED(i, &dma_ccr_buf[i * 24U]);
    }
    send_cursor = WS2812_LEDS_PER_DMA_IRQ;

    /* 填后半缓冲: LED 4-7 */
    for (uint8_t i = 0U; i < WS2812_LEDS_PER_DMA_IRQ; i++) {
        WS2812_EncodeLED(send_cursor, &dma_ccr_buf[WS2812_LEDS_PER_DMA_IRQ * 24U + i * 24U]);
        send_cursor++;
    }
}

/*============================================================================
 * HAL中断回调 (覆盖HAL库__weak默认实现)
 *============================================================================*/

/**
 * @brief  DMA半传输完成回调 (ISR上下文)
 * @note   仅置标志, 不做重负载工作
 */
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        half_flag |= 0x01U;
    }
}

/**
 * @brief  DMA全传输完成回调 (ISR上下文)
 * @note   仅置标志, 不做重负载工作
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        half_flag |= 0x02U;
    }
}

/*============================================================================
 * 命令处理函数 (供Single_action动作表使用)
 *============================================================================*/

/**
 * @brief  LED全灯呼吸测试 (非阻塞状态机)
 * @param  param  未使用
 * @note   每20ms自动切换颜色: 红→绿→蓝→白→灭→循环
 */
void WS2812_TestCmd(char *param)
{
    (void)param;

    static uint32_t last_tick = 0U;
    static uint8_t  phase     = 0U;
    uint32_t now = HAL_GetTick();

    if ((now - last_tick) < 20U) {
        return;
    }
    last_tick = now;

    switch (phase) {
    case 0: WS2812_Fill(255, 0, 0);   break;  /* 全红 */
    case 1: WS2812_Fill(0, 255, 0);   break;  /* 全绿 */
    case 2: WS2812_Fill(0, 0, 255);   break;  /* 全蓝 */
    case 3: WS2812_Fill(255, 255, 255); break; /* 全白 */
    case 4: WS2812_Clear();           break;  /* 全灭 */
    default: break;
    }
    phase = (phase + 1U) % 5U;
}

/**
 * @brief  填充全部LED为指定RGB颜色
 * @param  param  参数字符串 "R G B" (如 "255 0 0")
 * @note   前缀匹配命令 "led_fill 255 0 0"
 */
void WS2812_FillCmd(char *param)
{
    int r = 0, g = 0, b = 0;
    if (sscanf(param, "%d %d %d", &r, &g, &b) == 3) {
        WS2812_Fill((uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
}

/**
 * @brief  熄灭全部LED
 * @param  param  未使用
 * @note   完全匹配命令 "led_off"
 */
void WS2812_OffCmd(char *param)
{
    (void)param;
    WS2812_Clear();
}
