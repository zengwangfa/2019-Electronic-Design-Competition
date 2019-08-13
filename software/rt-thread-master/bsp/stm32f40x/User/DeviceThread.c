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
#include "DataProcess.h"
#include "HMI.h"
#include "drv_i2c.h"
#include "FDC2214.h"
#include "my2490.h"
#include "nbiot.h"



void fdc2214_thread_entry(void *parameter)//高电平1.5ms 总周期20ms  占空比7.5% volatil
{

		rt_thread_mdelay(1000);

		rt_thread_mdelay(2000);
		uart_send_hmi_reboot();//让HMI串口屏复位
		uart_send_hmi_reboot();//发送两次 确保重启
		DataSubsection(Cap_Division,FDC2214_Data_In_Flash,50);//读取Flash中的数据，分割好 电容区间
	
		while(1)
		{
				switch(HMI_Status_Flag){
						case 0x01:/* 校准模式 */

											FDC2214_Data_Adjust(); //数据校准	
											break;
						case 0x02:/* 校准模式 */

											FDC2214_Data_Adjust(); //数据校准	
											break;
						case 0x03:/* 工作模式 */
											Capcity_Paper_Detection(); //获取电容值
											break;			
						case 0x04:/* 拓展功能页面选择 */
											rt_thread_mdelay(100);
											break;																									
						case 0x05:/* 打印机 纸张检测 */
											Printer_Paper_Detection();
											break;
																								
						case 0x06:/* 材料 检测 */
											Material_Detection();
											break;
						case 0x07:/* 纸币检测 */
											Money_Detection();
											break;
						case 0x08:/* 分段参数 */
											rt_thread_mdelay(100);
											break;
						case 0xFF:
											rt_thread_mdelay(100);
											break;
				}
				rt_thread_mdelay(3);
	
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
                    8,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 1ms

    if (fdc2214_tid != RT_NULL){
			
				IIC_Init(); /* 初始化 */
				rt_thread_mdelay(100);
				FDC2214_Init();
			
				rt_thread_startup(fdc2214_tid);
		}

		return 0;
}
INIT_APP_EXPORT(fdc2214_thread_init);

































