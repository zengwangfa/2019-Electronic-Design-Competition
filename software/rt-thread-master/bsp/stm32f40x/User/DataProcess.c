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
unsigned int CH0_DATA,CH1_DATA,CH2_DATA,CH3_DATA;//通道值

float ShortValue1,ShortValue2,ShortValue3,ShortValue4;//短路值

float res1,res2,res3,res4;//电容暂存值

float res1_arr[10],res2_arr[10],res3_arr[10],res4_arr[10];//电容暂存值
int PaperNumber = 0;      //纸张数量
int ShortFlag = 0;        //短路标志位

//判断data是否在 value ± range内
int is_in_range(float data,float value,float range)
{
		if( (data < (value+range) && data > (value-range))){
				return 1;
		}		
		return 0;
}



void get_capcity_value(void)
{
		static uint8 ON_OFF = 0;
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();

		FDC2214_GetChannelData(FDC2214_Channel_0, &CH0_DATA);//获取电容
		FDC2214_GetChannelData(FDC2214_Channel_1, &CH1_DATA);
		//FDC2214_GetChannelData(FDC2214_Channel_2, &CH2_DATA);
		//FDC2214_GetChannelData(FDC2214_Channel_3, &CH3_DATA);
		
		res1 = Cap_Calculate(&CH0_DATA);//电容赋值
		res2 = Cap_Calculate(&CH1_DATA);
		res3 = Cap_Calculate(&CH2_DATA);
		res4 = Cap_Calculate(&CH3_DATA);

	
		res1 = KalmanFilter1(&res1); //数据进行卡尔曼滤波
		res2 = KalmanFilter2(&res2);
		//res3 = KalmanFilter3(&res3);
		//res4 = KalmanFilter4(&res4);
	
		/* 调度器解锁 */	
		rt_exit_critical();

		if(ON_OFF == 0 && res1 != 0.0f){/* 初始化 获取短路值*/
				ON_OFF = 1;//自锁开关
			
				ShortValue1 = Cap_Calculate(&CH0_DATA);
				ShortValue2 = Cap_Calculate(&CH1_DATA);
				//ShortValue3 = Cap_Calculate(&CH2_DATA);
				//ShortValue4 = Cap_Calculate(&CH3_DATA);
		}
		
		
//		res1 = res1-temp1;//电容减去初始值 下面触摸板
//		res2 = res2-temp2;//上面触摸板
//		res3 = res3-temp3;//上面短排针
//		res4 = res4-temp4;//上面长排针
}

/* 获取上极板 容值*/
float get_top_capacity(void)
{
		return res1;

}
/* 获取下极板 容值 */
float get_bottom_capacity(void)
{
		return res2;

}
/* 短路判断 */
void Short_Circuit_Detection(void)
{
    //当数据在 短路数值范围变化
		if( (is_in_range(res1,ShortValue1,40.0f) &&  is_in_range(res2,ShortValue2,40.0f)) \
			|| (res1 < 1.0f) ||  (res2 < 1.0f) ){//或者
				ShortFlag = 1;//判定短路
				Bling_Set(&Light_Blue,200,100,0.5,0,79,0);//蓝灯提示短路
		}			
		else{
				Bling_Set(&Light_Green,200,100,0.5,0,78,0);//绿灯提示不短路
				ShortFlag = 2;
		}
}


void fdc2214_thread_entry(void *parameter)//高电平1.5ms 总周期20ms  占空比7.5% volatil
{
		rt_thread_mdelay(3000);
		while(1)
		{
			
				get_capcity_value();
				Short_Circuit_Detection();
				rt_thread_mdelay(2);
		}
	
}


int fdc2214_thread_init(void)
{
    rt_thread_t fdc2214_tid;
		/*创建动态线程*/
    fdc2214_tid = rt_thread_create("fdc2214",//线程名称
                    fdc2214_thread_entry,			 //线程入口函数【entry】
                    RT_NULL,							   //线程入口函数参数【parameter】
                    2048,										 //线程栈大小，单位是字节【byte】
                    5,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 1ms

    if (fdc2214_tid != RT_NULL){
				IIC_Init();
				rt_thread_mdelay(100);
				FDC2214_Init();
				rt_thread_startup(fdc2214_tid);
		}

		return 0;
}
INIT_APP_EXPORT(fdc2214_thread_init);






