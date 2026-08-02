/**
  * @file    Single_action.c
  * @brief   单一动作控制函数实现（命令表）
  */

#include "Single_action.h"

/**
 * @brief   向舵机发送动作数据
 */
static void Send_Data_To_Servo(const uint16_t data[][3],uint8_t count)
{
    for(uint8_t i=0;i<count;i++)
    {
        Servo_Write((uint8_t)data[i][0],(uint16_t)data[i][1],(uint16_t)data[i][2]);
    }
}

/**********************上位机获取与直接设置舵机指令*****************/
/**
  * @brief  设置舵机角度
  * @param  param: 参数字符串
  * @retval 无
  */
void Set_Servo_Pos(char *param)
{
    int servo_id,servo_angle,servo_time;
    if(sscanf(param,"%d %d %d",&servo_id,&servo_angle,&servo_time)==3)
    {
        Servo_Write((uint8_t)servo_id,(uint16_t)servo_angle,servo_time);
        printf("Set_Servo_Pos success:%d %d %d\n",servo_id,servo_angle,servo_time);
    }
    else{
        printf("Set_Servo_Pos error:%s\n",param);
    }
}

/**
  * @brief  读取舵机角度
  * @param  param: 参数字符串
  * @retval 舵机角度
  */
void Read_Servo_Pos(char *param)
{
    int servo_id;
    if(sscanf(param,"%d",&servo_id)==1)
    {
        uint16_t angle=Servo_ReadPos((uint8_t)servo_id);
        printf("Read_Servo_Pos success:%d\n",angle);
    }
    else{
        printf("Read_Servo_Pos error:%s\n",param);
    }
}

/**
  * @brief  读取所有舵机角度
  * @retval 无
  */
void ReadAllPos(char *param)
{
    int servo_id;
    for(servo_id=1;servo_id<=19;servo_id++)
    {
        uint16_t angle=Servo_ReadPos((uint8_t)servo_id);
        printf("Read_Servo_Pos success:%d %d\n",servo_id,angle);
        HAL_IWDG_Refresh(&hiwdg);   // 喂独立看门狗，防止复位,2048ms
    }
    printf("ReadAllPos success\n");
}

/**
 * @brief   重置所有舵机角度为默认
 */
void Reset_Whole(char *param)
{
    Send_Data_To_Servo(Reset_Whole_Data,Reset_Whole_Count);
    printf("Reset_Whole success\n");
}

/**
 * @brief   停止所有舵机动作
 */
void Stop(char *param)
{
    for(uint8_t id=1;id<=19;id++)
    {
        Servo_Stop(id);
    }
    printf("Stop success\n");
}

/**
 * @brief   软件归零（设置当前yaw为0度）
 * @retval  无
 */
void Zero_Yaw(char *param)
{
    Location_deal_ZeroYaw();//软件归零
    flag.zero_yaw = 1;    //设置软件归零标志为已归零
    printf("Zero_Yaw success\n");
}

/**
 * @brief   取消软件归零（设置当前yaw角度为默认值）
 * @retval  无
 */
void Off_Zero_Yaw(char *param)
{
    flag.zero_yaw = 0;    //设置软件归零标志为未归零
    Location_deal_ClearYawError();//清空软件归零与yaw角度偏差
    printf("Off_Zero_Yaw success\n");
}

/***********************动作数组定义*********************/
static const Action action[] = {
    {"reset_whole", 0, 0,Reset_Whole},
    {"read_all_pos",0,0,ReadAllPos},
    {"set ",1,0,Set_Servo_Pos},
    {"read ",1,0,Read_Servo_Pos},
    {"stop",0,0,Stop},
    {"zero_yaw",0,0,Zero_Yaw},
    {"off_zero_yaw",0,0,Off_Zero_Yaw},
}; // 动作表，存放所有动作的名称和对应的函数指针

/**************查找动作名称字符串在动作表中的位置***********/
/**
  * @brief  根据动作名称执行相应动作的命令表
  * @param  name: 动作名称字符串
  * @retval 无
  */
void Single_Action(char *name) // 根据传入的动作名称字符串，在动作表中查找对应的函数指针并执行相应动作
{
  for (int i = 0; i < ACTION_COUNT; i++) {
    if (action[i].is_circular == 0) // 如果是单次动作
    {
      if (action[i].is_prefix == 0) // 如果是完全匹配
      {
        if (strcmp(action[i].name, name) == 0) {
          action[i].handler(NULL);
          return;
        }
      } else if (action[i].is_prefix == 1) // 如果是前缀匹配
      {
        uint8_t len = strlen(action[i].name);
        if (strncmp((const char *)action[i].name, name, len) == 0) {
          action[i].handler(name + len); // 将指令前缀之后的“参数部分”传递给业务函数
          return;
        }
      }
    } else if (action[i].is_circular == 1) // 如果是循环动作
    {
    }
  }
  printf("unknown single action:%s\n", name);
}
