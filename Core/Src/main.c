/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "string.h"
#include "mpu6050_dmp.h"
#include "LX-16A.h"
#include "Single_action.h"
#include "Task.h"
#include "Circular_dance.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
    KEY_UP = 0,   //按键未按下状态
    KEY_DOWN,   //按键按下状态
    KEY_STAY    //按键保持状态
} KeyState;
KeyState key_state = KEY_UP;    //按键状态变量，初始为未按下状态

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile Mode g_mode = DEBUG;    //动作模式枚举变量，初始为调试模式

volatile Flag flag = {0};    //动作执行状态标志变量，初始为未执行状态

volatile uint32_t tick = 0;    //时间截，单位毫秒

volatile float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;    //欧拉角，单位度

volatile uint8_t uart1_rx_buf[UART1_RX_SIZE];   // USART1 接收缓冲区，64 字节
volatile uint8_t uart6_rx_buf[UART6_RX_SIZE];   // USART6 接收缓冲区，30 字节

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_USART6_UART_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim10);     //启动 TIM10
  HAL_TIM_Base_Start_IT(&htim11);     //启动 TIM11
  HAL_TIM_Base_Start_IT(&htim4);      //启动 TIM4
  HAL_TIM_Base_Start_IT(&htim5);      //启动 TIM5
  
  SEGGER_RTT_Init ();   //J-Link RTT 初始化

HAL_UARTEx_ReceiveToIdle_DMA(&huart1,(uint8_t*) uart1_rx_buf, sizeof(uart1_rx_buf));   //开启 USART1 的 DMA 接收，接收数据存入 uart1_rx_buf
HAL_UARTEx_ReceiveToIdle_DMA(&huart6, (uint8_t*)uart6_rx_buf, sizeof(uart6_rx_buf));   //开启 USART6 的 DMA 接收，接收数据存入 uart6_rx_buf

/* 陀螺仪初始化 — 带总线恢复 + 最多 3 次重试 */
  int dmp_ok = 0;
  for (int retry = 1; retry <= 3; retry++)
  {
      if (retry > 1) {
          (g_mode==DEBUG) && SEGGER_RTT_printf(0, "DMP retry %d/3 - bus recovery...\n", retry);
          MPU6050_I2C_BusRecovery();    //I2C总线恢复
          MX_I2C1_Init();   //重新初始化 I2C1
          HAL_Delay(200);
      }
      (g_mode==DEBUG) && SEGGER_RTT_printf(0, "DMP Init attempt %d/3...\n", retry);
      if (MPU6050_DMP_Init() == 0) {
          dmp_ok = 1;
          break;
      }
  }
  if (dmp_ok) {
      (g_mode==DEBUG) && SEGGER_RTT_printf(0, "DMP Init OK\n");
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 点亮 LED
      HAL_IWDG_Refresh(&hiwdg);   // 喂独立看门狗

      HAL_IWDG_Refresh(&hiwdg);
  } else {
      (g_mode==DEBUG) && SEGGER_RTT_printf(0, "DMP Init FAILED after 3 attempts\n");
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // 熄灭 LED
      while(1) ;    // 3 次都失败，饿死看门狗复位
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if(flag.DMA_Send == 1)
    {
      Single_Action((char*)uart6_rx_buf);   //调用动作函数
      (g_mode==DEBUG) && SEGGER_RTT_printf(0,"[DMA] USART6 received, executing action: %s\n", uart6_rx_buf);
      flag.DMA_Send = 0;    //动作执行完成后，将标志位设为未执行状态
    }

    if(flag.mpu6050_data_ready==1)
    {
      if (MPU6050_DMP_GetEuler((float*)&pitch, (float*)&roll, (float*)&yaw) == 0)//强制转化为 float* 类型
    {
        /* SEGGER_RTT_printf 不支持 %f, 用整数+小数方式打印 */
        int p_int = (int)pitch;
        int p_frac = (int)((pitch > 0 ? pitch : -pitch) * 100.0f) % 100;
        int r_int = (int)roll;
        int r_frac = (int)((roll > 0 ? roll : -roll) * 100.0f) % 100;
        int y_int = (int)yaw;
        int y_frac = (int)((yaw > 0 ? yaw : -yaw) * 100.0f) % 100;
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "[%lu] P:%d.%02d R:%d.%02d Y:%d.%02d\n",
                          HAL_GetTick(), p_int, p_frac, r_int, r_frac, y_int, y_frac);
        flag.mpu6050_data_ready=0;    //数据处理完成后，将标志位设为未接收状态
    }
    }
    /*for(int i=0;i<500;i++)
    {
Servo_Write(1,i,0);
HAL_Delay(1);
(g_mode==DEBUG) && SEGGER_RTT_printf(0,"============================%d",i);
    }*/
// 获取当前系统运行的毫秒级时间戳
      uint32_t current_time = HAL_GetTick(); 

      // 遍历任务表
      for (int i = 0; i < task_count; i++) 
      {
          // 如果任务是激活状态
          if (task_table[i].is_active) 
          {
              // 计算时间差：当前时间 - 上次执行时间 >= 任务周期
              if (current_time - task_table[i].last_run_time >= task_table[i].interval_ms) 
              {
                  task_table[i].task_func();                   // 1. 执行任务
                  task_table[i].last_run_time = HAL_GetTick();  // 2. 更新最后执行时间
              }
          }
      }
(g_mode==DEBUG) && SEGGER_RTT_printf(0,"Tick:%d\n",HAL_GetTick());
    HAL_IWDG_Refresh(&hiwdg);   // 喂独立看门狗，防止复位,2048ms
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_15)    //如果接收到数据，置标志位（DMP中断），10ms中断一次
    {
        flag.mpu6050_data_ready = 1;
        (g_mode==DEBUG) && SEGGER_RTT_printf(0, "MPU6050\nData\nReady!\n");
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        
    }
    if (huart->Instance == USART6)
    {
        uart6_rx_buf[Size] = '\0';   //确保字符串以 null 结尾
        flag.DMA_Send=1;    //DMA发送标志位为1，表示数据已接收完毕
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, (uint8_t*)uart6_rx_buf, sizeof(uart6_rx_buf));  //重新开启DMA接收
    }
  }

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // DMA 发送完成，从 FIFO 取下一包继续发
        if (Fifo_Read(&fifo, fifo_packet))
        {
            HAL_UART_Transmit_DMA(&huart1, fifo_packet, 10);
        }
        // FIFO 空了→自动停止，等待下次 Servo_Write 触发
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM10)    //20ms 中断一次，读取按键状态
    {
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==GPIO_PIN_SET)   //按键按下
        {
            switch (key_state)
            {
                case KEY_UP:
                    key_state = KEY_DOWN;
                    break;
                case KEY_DOWN:
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 闪烁 LED
                key_state = KEY_STAY;
                    break;
                case KEY_STAY:
                    break;
                default:
                    key_state = KEY_UP;
                    break;
            }
    }
        else    //按键松开
        {
            key_state = KEY_UP;
        }
  }

    if (htim->Instance == TIM11)    //15ms 中断一次
    {
        
    }

    if (htim->Instance == TIM4)     //100ms 中断一次，实现较长时间的非阻滞延时
    {
        
    }

    if (htim->Instance == TIM5)     //10ms 中断一次，实现较短时间的非阻滞延时
    {
        
    }
  }

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
