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
};

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

/**************薄函数，供外部调用***************/
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
static void Set_Move_Flag(char name[30])     //根据传入的动作名称字符串，在动作表中查找对应的循环动作并设置相应的动作执行标志
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
    memset(name, 0, 30);   //清空传入的动作名称字符串，避免重复设置同一动作的执行标志
}

/**********************执行动作函数*****************/
void Take_Action(const uint16_t a[][3], uint8_t count)        //根据传入的动作参数数组，依次调用舵机控制函数执行相应动作
{
    for (int i = 0; i < count; i++)
    {
        Servo_Write((uint8_t)a[i][0], a[i][1], a[i][2]);
    }
}

/***********************动作数组定义*********************/
static const Action action[]=
{
    {"reset_whole",Reset_Whole_Data,sizeof(Reset_Whole_Data)/sizeof(Reset_Whole_Data[0]),0},
    {"left_hand_up",Left_Hand_Up_Data,sizeof(Left_Hand_Up_Data)/sizeof(Left_Hand_Up_Data[0]),0},
    {"left_hand_down",Left_Hand_Down_Data,sizeof(Left_Hand_Down_Data)/sizeof(Left_Hand_Down_Data[0]),0},
    {"right_hand_up",Right_Hand_Up_Data,sizeof(Right_Hand_Up_Data)/sizeof(Right_Hand_Up_Data[0]),0},
    {"right_hand_down",Right_Hand_Down_Data,sizeof(Right_Hand_Down_Data)/sizeof(Right_Hand_Down_Data[0]),0},
    {"left_step_forward",Left_Step_Forward_Data,sizeof(Left_Step_Forward_Data)/sizeof(Left_Step_Forward_Data[0]),0},
    {"right_step_forward",Right_Step_Forward_Data,sizeof(Right_Step_Forward_Data)/sizeof(Right_Step_Forward_Data[0]),0},
    {"left_step_backward",Left_Step_Backward_Data,sizeof(Left_Step_Backward_Data)/sizeof(Left_Step_Backward_Data[0]),0},
    {"right_step_backward",Right_Step_Backward_Data,sizeof(Right_Step_Backward_Data)/sizeof(Right_Step_Backward_Data[0]),0},
    {"left_moveaside",Left_MoveAside_Data,sizeof(Left_MoveAside_Data)/sizeof(Left_MoveAside_Data[0]),0},
    {"right_moveaside",Right_MoveAside_Data,sizeof(Right_MoveAside_Data)/sizeof(Right_MoveAside_Data[0]),0},
    {"left_turn",Left_Turn_Data,sizeof(Left_Turn_Data)/sizeof(Left_Turn_Data[0]),0},
    {"right_turn",Right_Turn_Data,sizeof(Right_Turn_Data)/sizeof(Right_Turn_Data[0]),0},
    {"stand_withleft",Stand_WithLeft_Data,sizeof(Stand_WithLeft_Data)/sizeof(Stand_WithLeft_Data[0]),0},
    {"stand_withright",Stand_WithRight_Data,sizeof(Stand_WithRight_Data)/sizeof(Stand_WithRight_Data[0]),0},
    {"walk_forward",NULL,0,1},
    {"walk_backward",NULL,0,1},
    {"move_toleft",NULL,0,1},
    {"move_toright",NULL,0,1},
    {"stand_likedazhi",Stand_LikeDaZi_Data,sizeof(Stand_LikeDaZi_Data)/sizeof(Stand_LikeDaZi_Data[0]),0},
    {"stand_likegongjian",Stand_LikeGongJian_Data,sizeof(Stand_LikeGongJian_Data)/sizeof(Stand_LikeGongJian_Data[0]),0},
    {"squat",Squat_Data,sizeof(Squat_Data)/sizeof(Squat_Data[0]),0}
};      //动作表，存放所有动作的名称和对应的函数指针

const uint8_t action_count=sizeof(action)/sizeof(action[0]);        //计算动作数量

/**************查找动作名称字符串在动作表中的位置***********/
void Single_Action(char name[30])       //根据传入的动作名称字符串，在动作表中查找对应的函数指针并执行相应动作
{
    for (int i = 0; i < action_count; i++)
    {
        if (strcmp(action[i].name, name) == 0)
        {
         if(action[i].is_circular==0)     //如果是单次动作，直接调用函数执行
         {
            Take_Action(action[i].data, action[i].count);
            memset(name, 0, 30);   //清空传入的动作名称字符串，避免重复执行同一动作
            return;
        }
      else
      {
         Set_Move_Flag(name);     //如果是循环动作，调用函数设置相应的动作执行标志，由移动任务函数 Move_Action 根据标志值循环执行相应动作
         return;
      }
   }
    }
}
