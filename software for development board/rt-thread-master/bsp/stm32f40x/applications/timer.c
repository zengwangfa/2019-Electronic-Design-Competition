/*
 * timer.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  定时器
 */
#define LOG_TAG    "timer"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"
#include "flash.h"
#include <rtthread.h>
#include <elog.h>
#include "drv_MS5837.h"
#include "drv_ano.h"
#include "Control.h"
#include "gyroscope.h"
#include "debug.h"
#include "ret_data.h"
#include "PropellerControl.h"

/*---------------------- Constant / Macro Definitions -----------------------*/



/*----------------------- Variable Declarations -----------------------------*/




/*----------------------- Function Implement --------------------------------*/


//void TIM3_Int_Init(u16 arr,u16 psc)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
//	
//  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
//	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
//	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
//	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
//	
//	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
//	
//	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
//	TIM_Cmd(TIM3,DISABLE); //使能定时器3
//	
//	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //子优先级3
//	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	
//}

void TIM3_ENABLE(void)
{
	TIM_Cmd(TIM3,ENABLE); //使能定时器3

}

//static int cnt = 0;
//int timer3_cnt = 0;

//定时器3中断服务函数
//void TIM3_IRQHandler(void)
//{

//		if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
//		{
//			
//				ANO_SEND_StateMachine();
////				cnt ++;
////				if(cnt >= timer3_cnt){
////					  cnt = 0;
////						Two_Axis_Yuntai_Control();
////				}

//			
//		}
//		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
//}


//static int timer3_speed(int argc, char **argv)
//{
//    int result = 0;
//    if (argc != 2){
//        log_e("Error! Proper Usage: speed <0~100>");
//				result = -RT_ERROR;
//        goto _exit;
//    }
//		if(atoi(argv[1]) <= 1000){
//				timer3_cnt = atoi(argv[1]);
//		}
//		else {
//				log_e("Error! The value is out of range!");
//		}
//_exit:
//    return result;
//}
//MSH_CMD_EXPORT(timer3_speed,ag: speed  <0~100>);





/**
  * @brief  timer1_out(定时器中断函数)
  * @param  void* parameter
  * @retval None
  * @notice Attention! -> 定时器中不能存在延时或者释放线程的操作
  */
static void timer1_out(void* parameter)// 定时器1超时函数  进行JY901模块数据转换
{


		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();
				

	
//		get_speed(&Sensor.JY901.Acc.x,&Sensor.JY901.Speed.x);//得到x速度,数据有问题，累积效应过大
//		get_zspeed(); 
		//Angle_Control(); //角度控制


		/* 调度器解锁 */
		rt_exit_critical();

	
}



int timer1_init(void)
{
		/* 定时器的控制块 */
		static rt_timer_t timer1;
    /* 创建定时器1 */
    timer1 = rt_timer_create("timer1",  /* 定时器名字是 timer1 */
                        timer1_out, 		  /* 超时时回调的处理函数 */
                        RT_NULL, 			  /* 超时函数的入口参数 */
                        1,      			  /* 定时长度，以OS Tick为单位，即5个OS Tick   --> 50MS*/  
                        RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_HARD_TIMER); /* 周期性定时器 */
    /* 启动定时器 */
    if (timer1 != RT_NULL){ 
				//TIM3_Int_Init(10000-1,84-1); //84M/84 = 1M,  1M/10000 = 100Hz  = 10MS
				//rt_timer_start(timer1);
				
		}

    return 0;
}
//INIT_APP_EXPORT(timer1_init);

