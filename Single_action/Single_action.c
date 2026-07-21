/**
  * @file    Single_action.c
  * @brief   单一动作控制函数实现（命令表）
  */

#include "Single_action.h"
#include "string.h"

/*************动作数据表*******************/

const uint16_t Reset_Whole_Data[][3]=
{
   {1, 500, 0},  {2, 500, 0},  {3, 500, 0},  {4, 500, 0},
    {5, 500, 0},  {6, 500, 0},  {7, 500, 0},  {8, 500, 0},
    {9, 500, 0},  {10, 500, 0}, {11, 500, 0}, {12, 500, 0},
    {13, 500, 0}, {14, 500, 0}, {15, 500, 0}, {16, 500, 0},
    {17, 500, 0}, {18, 500, 0}
};      //所有舵机复位

static const uint16_t Left_Hand_Up_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Left_Hand_Down_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_Hand_Up_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_Hand_Down_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Left_Step_Forward_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_Step_Forward_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Left_Step_Backward_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_Step_Backward_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Left_MoveAside_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_MoveAside_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Left_Turn_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Right_Turn_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Stand_WithLeft_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Stand_WithRight_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Stand_LikeDaZi_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Stand_LikeGongJian_Data[][3]=
{
    {1,500,1000}
};

static const uint16_t Squat_Data[][3]=
{
    {1,500,1000}
};

/*****************串口调试指令*****************/
/*[s %d %d],控制舵机角度,第一个参数为舵机编号,第二个参数为角度值*/
/**
  * @brief  控制舵机角度
  * @param  param: 参数字符串
  * @retval 无
  */
void Slider(char *param)
{
    int slider_id,slider_angle;
    sscanf(param,"%d %d]",&slider_id,&slider_angle);
    Servo_Write((uint8_t)slider_id,(uint16_t)slider_angle,0);
}

extern uint8_t pos_read_id;
void ReadAllPos(char *param)
{
    pos_read_id=1;    //位置读取 ID，初始为 1，范围 1-19
    Servo_ReadPos(pos_read_id);
}

/**************薄函数，供外部调用***************/
/**
  * @brief  复位所有舵机
  * @retval 无
  */
void Reset_Whole(void)
{
Take_Action(Reset_Whole_Data,sizeof(Reset_Whole_Data)/sizeof(Reset_Whole_Data[0]));
}

void Left_Step_Forward(void)     //左迈步
{
Take_Action(Left_Step_Forward_Data,sizeof(Left_Step_Forward_Data)/sizeof(Left_Step_Forward_Data[0]));
}

void Right_Step_Forward(void)     //右迈步
{
Take_Action(Right_Step_Forward_Data,sizeof(Right_Step_Forward_Data)/sizeof(Right_Step_Forward_Data[0]));
}

void Left_Step_Backward(void)
{
Take_Action(Left_Step_Backward_Data,sizeof(Left_Step_Backward_Data)/sizeof(Left_Step_Backward_Data[0]));
}

void Right_Step_Backward(void)     
{
Take_Action(Right_Step_Backward_Data,sizeof(Right_Step_Backward_Data)/sizeof(Right_Step_Backward_Data[0]));
}

void Left_MoveAside(void)     
{
Take_Action(Left_MoveAside_Data,sizeof(Left_MoveAside_Data)/sizeof(Left_MoveAside_Data[0]));
}

void Right_MoveAside(void)     
{
Take_Action(Right_MoveAside_Data,sizeof(Right_MoveAside_Data)/sizeof(Right_MoveAside_Data[0]));
}

/***************设置动作执行标志*****************/

extern uint8_t move_action_active;

/**
  * @brief  设置移动动作标志
  * @param  name: 动作名称字符串
  * @retval 无
  */
static void Set_Move_Flag(char name[UART6_RX_SIZE])     //根据传入的动作名称字符串，在动作表中查找对应的循环动作并设置相应的动作执行标志
{
    if(strcmp(name,"walk_forward")==0)
    {
        flag.walk_forward=1;
    }
    else if(strcmp(name,"walk_backward")==0)
    {
        flag.walk_backward=1;
    }
    else if(strcmp(name,"move_toleft")==0)
    {
        flag.move_to_left=1;
    }
    else if(strcmp(name,"move_toright")==0)
    {
        flag.move_to_right=1;
    }
    memset(name, 0, UART6_RX_SIZE);   //清空传入的动作名称字符串，避免重复设置同一动作的执行标志
    move_action_active=1;   //激活移动任务
}

/**********************执行动作函数*****************/
/**
  * @brief  执行动作
  * @param  a: 动作参数数组
  * @param  count: 动作数量
  * @retval 无
  */
void Take_Action(const uint16_t a[][3], uint8_t count)        //根据传入的动作参数数组，依次调用舵机控制函数执行相应动作
{
    for (int i = 0; i < count; i++)
    {
        Servo_Write((uint8_t)a[i][0], a[i][1], a[i][2]);
    }
}

/***********************动作数组定义*********************/
static const Action action[] = {
    {"reset_whole", 0, Reset_Whole_Data,
     sizeof(Reset_Whole_Data) / sizeof(Reset_Whole_Data[0]), 0,NULL},
    {"left_hand_up", 0, Left_Hand_Up_Data,
     sizeof(Left_Hand_Up_Data) / sizeof(Left_Hand_Up_Data[0]), 0,NULL},
    {"left_hand_down", 0, Left_Hand_Down_Data,
     sizeof(Left_Hand_Down_Data) / sizeof(Left_Hand_Down_Data[0]), 0,NULL},
    {"right_hand_up", 0, Right_Hand_Up_Data,
     sizeof(Right_Hand_Up_Data) / sizeof(Right_Hand_Up_Data[0]), 0,NULL},
    {"right_hand_down", 0, Right_Hand_Down_Data,
     sizeof(Right_Hand_Down_Data) / sizeof(Right_Hand_Down_Data[0]), 0,NULL},
    {"left_step_forward", 0, Left_Step_Forward_Data,
     sizeof(Left_Step_Forward_Data) / sizeof(Left_Step_Forward_Data[0]), 0,NULL},
    {"right_step_forward", 0, Right_Step_Forward_Data,
     sizeof(Right_Step_Forward_Data) / sizeof(Right_Step_Forward_Data[0]), 0,NULL},
    {"left_step_backward", 0, Left_Step_Backward_Data,
     sizeof(Left_Step_Backward_Data) / sizeof(Left_Step_Backward_Data[0]), 0,NULL},
    {"right_step_backward", 0, Right_Step_Backward_Data,
     sizeof(Right_Step_Backward_Data) / sizeof(Right_Step_Backward_Data[0]), 0,NULL},
    {"left_moveaside", 0, Left_MoveAside_Data,
     sizeof(Left_MoveAside_Data) / sizeof(Left_MoveAside_Data[0]), 0,NULL},
    {"right_moveaside", 0, Right_MoveAside_Data,
     sizeof(Right_MoveAside_Data) / sizeof(Right_MoveAside_Data[0]), 0,NULL},
    {"left_turn", 0, Left_Turn_Data,
     sizeof(Left_Turn_Data) / sizeof(Left_Turn_Data[0]), 0,NULL},
    {"right_turn", 0, Right_Turn_Data,
     sizeof(Right_Turn_Data) / sizeof(Right_Turn_Data[0]), 0,NULL},
    {"stand_withleft", 0, Stand_WithLeft_Data,
     sizeof(Stand_WithLeft_Data) / sizeof(Stand_WithLeft_Data[0]), 0,NULL},
    {"stand_withright", 0, Stand_WithRight_Data,
     sizeof(Stand_WithRight_Data) / sizeof(Stand_WithRight_Data[0]), 0,NULL},
    {"walk_forward", 0, NULL, 0, 1,NULL},
    {"walk_backward", 0, NULL, 0, 1,NULL},
    {"move_toleft", 0, NULL, 0, 1,NULL},
    {"move_toright", 0, NULL, 0, 1,NULL},
    {"stand_likedazhi", 0, Stand_LikeDaZi_Data,
     sizeof(Stand_LikeDaZi_Data) / sizeof(Stand_LikeDaZi_Data[0]), 0,NULL},
    {"stand_likegongjian", 0, Stand_LikeGongJian_Data,
     sizeof(Stand_LikeGongJian_Data) / sizeof(Stand_LikeGongJian_Data[0]), 0,NULL},
    {"squat", 0, Squat_Data, sizeof(Squat_Data) / sizeof(Squat_Data[0]),0,NULL},
    {"[s ", 1, NULL,NULL,0,Slider},
    {"read_all_pos",0,NULL,NULL,0,ReadAllPos}
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
    if (action[i].is_prefix == 0) // 如果是完全匹配
    {
      if (action[i].is_circular == 0) // 如果是单次动作
      {
        if (strcmp(action[i].name, name) ==
            0) // 如果是单次动作，直接调用函数执行
        {
          Take_Action(action[i].data, action[i].count);
          return;
        }
      } else if (action[i].is_circular == 1) // 如果是循环动作
      {
        if (strcmp(action[i].name, name) == 0) {
          Set_Move_Flag(name);
          // 如果是循环动作，调用函数设置相应的动作执行标志，由移动任务函数
          // Move_Action 根据标志值循环执行相应动作
          return;
        }
      }
    } else if (action[i].is_prefix == 1) // 如果是前缀匹配
    {
      uint8_t len = strlen(action[i].name);
      if (strncmp((const char *)action[i].name, name, len) == 0) {
        action[i].handler(name +len); // 将指令前缀之后的“参数部分”传递给业务函数
        return;
      }
    }
  }
  printf("unknown single action:%s\n", name);
}
