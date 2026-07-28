/**
  * @file    Task.c
  * @brief   任务控制函数实现（TCB+调度器）
  */

#include "main.h"
#include "Task.h"

/**
  * @brief  移动动作周期与激活状态
  */
uint32_t move_action_interval_ms = 10;   //移动任务周期，10ms
uint8_t move_action_active = 0;   //移动任务默认不激活

/**
  * @brief  按键事件周期与激活状态，枚举定义
  */
uint32_t key_event_interval_ms = 13;   //按键任务周期，13ms
extern volatile Flag flag;    //动作执行状态标志变量，初始为未执行状态
KeyState key_state = KEY_UP;    //按键状态变量，初始为未按下状态

/**
  * @brief  节拍检测任务周期与激活状态
  */
uint32_t beat_task_interval_ms = 6;   //节拍任务周期，6ms

/**
  * @brief  当检测到节拍时翻转电平任务
  */
uint32_t beat_action_task_interval_ms = 11;   //节拍触发任务周期，11ms

/**
  * @brief  表驱动的时间触发合作式调度器
  */
TaskDef task_table[] = {
    {Move_Action,&move_action_interval_ms, 0, &move_action_active},
    {Key_Event,&key_event_interval_ms, 0,(uint8_t*) &flag.key_event},
    {I2S_Beat_Task,&beat_task_interval_ms, 0, (uint8_t*)&flag.beat_active},
    {I2S_Beat_Action,&beat_action_task_interval_ms, 0, (uint8_t*)&flag.beat_active},
};

const uint8_t task_count=sizeof(task_table) / sizeof(task_table[0]);// 任务表中任务的数量

/**
  * @brief  移动动作控制块，前后左右
  */
MoveAction move_actions[] = {
{&flag.walk_forward, Left_Step_Forward,Right_Step_Forward,1000, 3},   //前进动作，预定步数3，左右脚交替前踏，每步间隔1000ms
{&flag.walk_backward, Left_Step_Backward,Right_Step_Backward,1000, 3},   //后退动作，预定步数3，左右脚交替后撤，每步间隔1000ms
{&flag.move_to_left, Left_MoveAside,Reset_Whole,1000, 3},   //左移动作，预定步数3，左右脚交替向左移动，每步间隔1000ms
{&flag.move_to_right, Right_MoveAside,Reset_Whole,1000, 3}    //右移动作，预定步数3，左右脚交替向右移动，每步间隔1000ms
};

/**
  * @brief  执行移动动作
  * @retval 无
  */
void Move_Action(void)
{
 static enum{IDLE,LEFT_STEP,WAIT_LEFT,RIGHT_STEP,WAIT_RIGHT} state = IDLE;   //定义状态机状态
 
 static uint8_t member=0;    //记录当前执行的动作成员，0-前进，1-后退，2-左移，3-右移

 static uint32_t step_last_time = 0;     //记录步态切换的时间

 static uint8_t action_count = 0;       //记录已执行的步数

 switch (state) {
 case IDLE:
   for (int i = 0; i < ACTION_NUM;i++) // 遍历动作表，检查哪个动作的执行标志被置位
   {
     if (*(move_actions[i].move_flag) == 1) // 接收到动作指令，进入状态机
     {
       member = i; // 记录当前执行的动作成员
       action_count = move_actions[i].count; // 预定步数
       state = LEFT_STEP;
       break; // 找到一个被置位的动作后就跳出循环，优先执行第一个被置位的动作
     }
   }
   break;
 case LEFT_STEP:
   move_actions[member].task_func_1(); // 执行左脚动作
   step_last_time = HAL_GetTick();
   state = WAIT_LEFT;
   break;
 case WAIT_LEFT:
   if (HAL_GetTick() - step_last_time >=
       move_actions[member].wait_time_ms) // 等待，确保左脚动作完成
   {
     state = RIGHT_STEP;
   }
   break;
 case RIGHT_STEP:
   move_actions[member].task_func_2(); // 执行右脚动作
   step_last_time = HAL_GetTick();
   state = WAIT_RIGHT;
   break;
 case WAIT_RIGHT:
   if (HAL_GetTick() - step_last_time >=
       move_actions[member].wait_time_ms) // 等待，确保右脚动作完成
   {
     action_count--;     // 步数递减
     step_last_time = 0; // 重置步态切换时间
     if (action_count > 0) {
       state = LEFT_STEP; // 循环执行左右脚交替前踏动作
     } else {
       Reset_Whole(); // 完成预定步数后复位全身动作
       state = IDLE;  // 完成预定步数后回到空闲状态
       *(move_actions[member].move_flag) = 0; // 重置动作执行标志
     }
   }
   break;
 }
}

/**
  * @brief  处理按键事件,当flag.key_event为1时,15ms执行一次
  * @retval 无
  */
void Key_Event(void)
{
    switch (key_state)      // 根据按钮事件状态进行处理
    {
        case KEY_UP:
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                key_state = KEY_DOWN;
            } else {
                flag.key_event = 0;   // 误触发，清除标志
            }
            break;
        case KEY_DOWN:
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                key_state = KEY_STAY;
            } else {
                key_state = KEY_UP;
                flag.key_event = 0;   // 短暂按下后松开，结束
            }
            break;
        case KEY_STAY:
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
                key_state = KEY_UP;
                flag.key_event = 0;   // 松开，完整周期结束
            }
            break;
    }
}

/**
  * @brief  处理节拍动作任务
  * @retval 无
  */
void I2S_Beat_Action(void)
{
  static uint8_t last_beat = 0;   //记录上一次节拍状态
  if(I2S_Beat_IsBeat() && !last_beat)   //检测到节拍触发瞬间
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
  last_beat = I2S_Beat_IsBeat();   //更新上一次节拍状态
}

/**
  * @brief  处理任务表
  * @retval 无
  */
void Task_Process(void)
{
    for (uint8_t i = 0; i < task_count; i++)      // 遍历任务列表
    {
        if (*task_table[i].is_active==1)      // 如果任务已激活
        {
            if (HAL_GetTick() - task_table[i].last_run_time >= *task_table[i].interval_ms)      // 如果时间到
            {
                task_table[i].last_run_time = HAL_GetTick();      // 更新上次运行时间
                task_table[i].task_func();      // 执行任务
            }
        }
    }
}
