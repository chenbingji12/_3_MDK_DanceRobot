#include "main.h"
#include "Task.h"

/*****************表驱动的时间触发合作式调度器******************/
TaskDef task_table[] = {
    {Move_Action,10, 0, 1}
};

const uint8_t task_count=sizeof(task_table) / sizeof(task_table[0]);

/**********************前后左右移动************************/
MoveAction move_actions[] = {
{&flag.walk_forward, Left_Step_Forward,Right_Step_Forward,1000, 3},   //前进动作，预定步数3，左右脚交替前踏，每步间隔1000ms
{&flag.walk_backward, Left_Step_Backward,Right_Step_Backward,1000, 3},   //后退动作，预定步数3，左右脚交替后撤，每步间隔1000ms
{&flag.move_to_left, Left_MoveAside,Reset_Whole,1000, 3},   //左移动作，预定步数3，左右脚交替向左移动，每步间隔1000ms
{&flag.move_to_right, Right_MoveAside,Reset_Whole,1000, 3}    //右移动作，预定步数3，左右脚交替向右移动，每步间隔1000ms
};

void Move_Action(void)
{
 static enum{IDLE,LEFT_STEP,WAIT_LEFT,RIGHT_STEP,WAIT_RIGHT} state = IDLE;   //定义状态机状态
 
 static uint8_t member=0;    //记录当前执行的动作成员，0-前进，1-后退，2-左移，3-右移

 static uint32_t step_last_time = 0;     //记录步态切换的时间

 static uint8_t action_count = 0;       //记录已执行的步数

    switch(state)
    {
        case IDLE:
        for(int i=0;i<ACTION_NUM;i++)     //遍历动作表，检查哪个动作的执行标志被置位
        {
            if(*(move_actions[i].move_flag)==1)        //接收到动作指令，进入状态机
            {
                member=i;     //记录当前执行的动作成员
                action_count=move_actions[i].count;    //预定步数
                state=LEFT_STEP;
                break;  //找到一个被置位的动作后就跳出循环，优先执行第一个被置位的动作
            }
        }
            break;
        case LEFT_STEP:
            move_actions[member].task_func_1();     //执行左脚动作
            step_last_time=HAL_GetTick();
            state=WAIT_LEFT;
            break;
        case WAIT_LEFT:
            if(HAL_GetTick()-step_last_time>=move_actions[member].wait_time_ms)   //等待，确保左脚动作完成
            {
                state=RIGHT_STEP;
            }
            break;
        case RIGHT_STEP:
            move_actions[member].task_func_2();     //执行右脚动作
            step_last_time=HAL_GetTick();
            state=WAIT_RIGHT;
            break;
        case WAIT_RIGHT:
            if(HAL_GetTick()-step_last_time>=move_actions[member].wait_time_ms)   //等待，确保右脚动作完成
            {
                action_count--;     //步数递减
                step_last_time=0;       //重置步态切换时间
                if(action_count>0)
                {
                state=LEFT_STEP;    //循环执行左右脚交替前踏动作
                }
                else
                {
                    Reset_Whole();     //完成预定步数后复位全身动作
                    state=IDLE;     //完成预定步数后回到空闲状态
                    *(move_actions[member].move_flag)=0;    //重置动作执行标志
                }
            }
            break;
    }
}
