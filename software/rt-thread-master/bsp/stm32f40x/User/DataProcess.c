/*
 * DataProcess.c
 *
 *  Created on: 2019年8月7日
 *      Author: zengwangfa
 *      Notes:  FDC2214数据处理
 */


#include "DataProcess.h"
#include "FDC2214.h"
#include <rtthread.h>
#include "filter.h"
#include "led.h"
#include "HMI.h"

unsigned int CH0_DATA,CH1_DATA,CH2_DATA,CH3_DATA;//通道值

float ShortValue1,ShortValue2,ShortValue3,ShortValue4;//短路值

float res1,res2,res3,res4;//电容暂存值

float res1_arr[10],res2_arr[10],res3_arr[10],res4_arr[10];//电容暂存值


PaperCountEngine_Type Paper = {
			.Finish_Flag = 2
};/*Paper数据结构体*/

extern int HMI_Status_Flag;

//判断data是否在 value ± range内
int is_in_range(float data,float value,float range)
{
		if( (data < (value+range) && data > (value-range))){
				return 1;
		}		
		return 0;
}


/* 获取电容值 */
void Get_Capcity_Value(void)
{
		static uint8 ON_OFF = 0;
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();

		FDC2214_GetChannelData(FDC2214_Channel_0, &CH0_DATA);//获取电容
		FDC2214_GetChannelData(FDC2214_Channel_1, &CH1_DATA);
		FDC2214_GetChannelData(FDC2214_Channel_2, &CH2_DATA);
		FDC2214_GetChannelData(FDC2214_Channel_3, &CH3_DATA);
		
		res1 = Cap_Calculate(&CH0_DATA);//电容赋值
		res2 = Cap_Calculate(&CH1_DATA);
		res3 = Cap_Calculate(&CH2_DATA);
		res4 = Cap_Calculate(&CH3_DATA);

	
		res1 = KalmanFilter1(&res1); //数据进行卡尔曼滤波
		res2 = KalmanFilter2(&res2);
		res3 = KalmanFilter3(&res3);
		res4 = KalmanFilter4(&res4);
	
		/* 调度器解锁 */	
		rt_exit_critical();

		Paper.Capacitance = res4;
		if(ON_OFF == 0 && res1 != 0.0f){/* 初始化 获取短路值*/
				ON_OFF = 1;//自锁开关
			
				ShortValue1 = Cap_Calculate(&CH0_DATA);
				ShortValue2 = Cap_Calculate(&CH1_DATA);
				ShortValue3 = Cap_Calculate(&CH2_DATA);
				ShortValue4 = Cap_Calculate(&CH3_DATA);
		}
		
		
//		res1 = res1-temp1;//电容减去初始值 下面触摸板
//		res2 = res2-temp2;//上面触摸板
//		res3 = res3-temp3;//上面短排针
//		res4 = res4-temp4;//上面长排针
}




/* 【校准时】获取上极板 容值*/
float get_top_capacity(void)
{
		static unsigned int res_CH0_DATA = 0;
		static float res_ch1_arr[10];
		static float res_ch1 = 0.0f;
		static int i = 0;
		
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();
	
		for(i = 0;i < 10;i++){
				FDC2214_GetChannelData(FDC2214_Channel_0, &res_CH0_DATA);//获取电容

				rt_thread_mdelay(10);
				res_ch1_arr[i] = Cap_Calculate(&res_CH0_DATA);//电容赋值

		}
		/* 调度器解锁 */	
		rt_exit_critical();
		
		res_ch1 = Bubble_Filter_Float(res_ch1_arr);
		return res_ch1;

}
/* 【校准时】获取下极板 容值 */
float get_bottom_capacity(void)
{

		static unsigned int res_CH1_DATA = 0;
		static float res_ch2_arr[10];
		static float res_ch2 = 0.0f;
		static int i = 0;
		
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();
	
		for(i = 0;i < 10;i++){
				FDC2214_GetChannelData(FDC2214_Channel_1, &res_CH1_DATA);

				rt_thread_mdelay(10);
				res_ch2_arr[i] = Cap_Calculate(&res_CH1_DATA);
		}
		/* 调度器解锁 */	
		rt_exit_critical();
		
		res_ch2 = Bubble_Filter_Float(res_ch2_arr);		
	
		return res_ch2;
}



/* 【校准时】获取单极板 容值 */
float get_single_capacity(void)
{

		static unsigned int res_CH4_DATA = 0;
		static float res_ch4_arr[10];
		static float res_ch4 = 0.0f;
		static int i = 0;
		
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();
	
		for(i = 0;i < 10;i++){
				FDC2214_GetChannelData(FDC2214_Channel_3, &res_CH4_DATA);

				rt_thread_mdelay(10);
				res_ch4_arr[i] = Cap_Calculate(&res_CH4_DATA);
		}
		/* 调度器解锁 */	
		rt_exit_critical();
		
		res_ch4 = Bubble_Filter_Float(res_ch4_arr);		
	
		return res_ch4;
}



/* 短路判断 */
void Short_Circuit_Detection(void)
{
    //当数据在 短路数值范围变化
//		if(  (is_in_range(res4,ShortValue4,40.0f)) || (is_in_range(res1,ShortValue1,40.0f) &&  is_in_range(res2,ShortValue2,40.0f)) \
//			|| (res1 < 1.0f) ||  (res2 < 1.0f)  ){//或者当值非常小的时候，判定为受到干扰
		if(  (is_in_range(res4,ShortValue4,40.0f)) || (res4 < 1.0f) ){//或者当值非常小的时候，判定为受到干扰
				Paper.ShortStatus = 1;//判定短路
				Bling_Set(&Light_Blue,200,100,0.5,0,79,0);//蓝灯提示短路
		}			
		else{
				Bling_Set(&Light_Green,200,100,0.5,0,78,0);//绿灯提示不短路
				Paper.ShortStatus = 2; //判定不短路
		}
		uart_send_hmi_is_short();
		
}



