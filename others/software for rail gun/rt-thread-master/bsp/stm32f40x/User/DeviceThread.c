/*
 * DeviceThread.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  设备控制任务
 */

#include "DeviceThread.h"
#include <rtthread.h>
#include <elog.h>
#include <math.h>
#include <stdlib.h>
#include "propeller.h"
#include "servo.h"
#include "light.h"
#include "rc_data.h"
#include "Control.h"
#include "PropellerControl.h"
#include "focus.h"
#include "debug.h"
#include "timer.h"
#include "gyroscope.h"
#include "PID.h"
#include "ioDevices.h"
#include "HMI.h"
uint8 thread_speed = 5;



void Set_Z_Zero(void)//设置Z轴归0
{
		/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
		rt_enter_critical();
	
		TIM4_PWM_CH1_D12(YAW_YUNTAI_MED);  //左中   D12
		TIM4_PWM_CH2_D13(PITCH_YUNTAI_MED); //右中   D13
		rt_thread_mdelay(2000);
	
		/* 调度器解锁 */
		rt_exit_critical();
	
		gyroscope_z_zero();

}
MSH_CMD_EXPORT(Set_Z_Zero,ag: Set_Z_Zero  );



/**
  * @brief  propeller_thread_entry(推进器控制任务函数)
  * @param  void* parameter
  * @retval None
  * @notice 
  */
void propeller_thread_entry(void *parameter)
{
		rt_thread_mdelay(1000);
		Set_Z_Zero();
		rt_thread_mdelay(1000);
		TIM3_ENABLE();
		while(1)
		{
				//drifting_check(); //漂移 检测			

				//Car_Pitch_Control();		
				//Servo_Dir_Control(0);			//舵机控制			
				Two_Axis_Yuntai_Control();
			

				
				rt_thread_mdelay(thread_speed); //5ms
		}
}




static int speed(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage: speed <0~100>");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 1000){
				thread_speed = atoi(argv[1]);
		}
		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(speed,ag: speed  <0~100>);








/**
  * @brief  servo_thread_entry(舵机控制任务函数)
  * @param  void* parameter
  * @retval None
  * @notice 
  */
void devices_thread_entry(void *parameter)//高电平1.5ms 总周期20ms  占空比7.5% volatil
{



		while(1)
		{
			
			
				
				if(0x01 == HMI_Work_Button){
						rail_gun_start_charge();
						rail_gun_start_up();	
						HMI_Work_Button = 0;
				}	
			//void rail_gun_start_charge(void)

//void rail_gun_start_up(void)
			
			
				if(WORK == WorkMode){//工作模式

						//Extractor_Control(&ControlCmd.Arm); //吸取器控制
						//RoboticArm_Control(&ControlCmd.Arm);//机械臂控制
//						Search_Light_Control(&ControlCmd.Light);  //探照灯控制
//						YunTai_Control(&ControlCmd.Yuntai); //云台控制				
//						Focus_Zoom_Camera_Control(&ControlCmd.Focus);//变焦摄像头控制					
				}
				else if(DEBUG == WorkMode)//调试模式
				{	
						//Debug_Mode(get_button_value(&ControlCmd));
				}
				rt_thread_mdelay(20);
		}
	
}


int propeller_thread_init(void)
{
    rt_thread_t propeller_tid;
		/*创建动态线程*/
    propeller_tid = rt_thread_create("propoller",//线程名称
                    propeller_thread_entry,			 //线程入口函数【entry】
                    RT_NULL,							   //线程入口函数参数【parameter】
                    2048,										 //线程栈大小，单位是字节【byte】
                    5,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 1ms

    if (propeller_tid != RT_NULL){
			
				PWM_Init(); //推进器、舵机类PWM初始化
			
				rt_thread_startup(propeller_tid);
		}

		return 0;
}
INIT_APP_EXPORT(propeller_thread_init);

int devices_thread_init(void)
{
    rt_thread_t devices_tid;
		/*创建动态线程*/
    devices_tid = rt_thread_create("devices",//线程名称
                    devices_thread_entry,			 //线程入口函数【entry】
                    RT_NULL,							   //线程入口函数参数【parameter】
                    1024,										 //线程栈大小，单位是字节【byte】
                    12,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 100ms

    if (devices_tid != RT_NULL){
				Light_PWM_Init(); //探照灯PWM初始化
				log_i("Light_init()");

				rt_thread_startup(devices_tid);

		}

		return 0;
}
INIT_APP_EXPORT(devices_thread_init);






