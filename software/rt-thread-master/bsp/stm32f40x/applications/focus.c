/*
 * focus.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  变焦摄像头设备(控制通信)
 */

#include "focus.h"
#include <rtthread.h>
#include <elog.h>
#include "uart.h"
#include "ret_data.h"
#include "Control.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
#define OPENMV_LEN 15

/*----------------------- Variable Declarations -----------------------------*/
uint8 focus_data_ok = 0;

uint8 addFocus_Data[6] = {0xAA,0x55,0x02,0x01,0x00,0x02}; //放大
uint8 minFocus_Data[6] = {0xAA,0x55,0x02,0x02,0x00,0x03}; //缩小

uint8 addZoom_Data[6]  = {0xAA,0x55,0x02,0x00,0x01,0x02}; //聚焦
uint8 minZoom_Data[6]  = {0xAA,0x55,0x02,0x00,0x02,0x03}; //放焦

uint8 Camera_Clear_Data[6] = {0xAA,0x55,0x02,0x88,0x88,0x11}; //恢复初始值
uint8 Camera_Stop_Data[6]  = {0xAA,0x55,0x02,0x00,0x00,0x01}; //停止
/*----------------------- Function Implement --------------------------------*/
uint8 focus_data[20] = {0};

int target_blob_x = 0;
int target_blob_y = 0;
int persent_blob_x = 0;
int persent_blob_y = 0;

/**
  * @brief  Focus_Zoom_Camera(摄像头变焦、放大)
  * @param  控制字符数据action [0x01聚焦、0x02放焦、0x11放大、0x12缩小]
  * @retval None
  * @notice 
  */
void Focus_Zoom_Camera_Control(uint8 *action)
{		
		switch(*action){
				case 0x01:rt_device_write(focus_uart_device, 0,addFocus_Data,   6); break;
				case 0x02:rt_device_write(focus_uart_device, 0,minFocus_Data,   6); break;
				case 0x11:rt_device_write(focus_uart_device, 0,addZoom_Data,    6); break;
				case 0x12:rt_device_write(focus_uart_device, 0,minZoom_Data,    6); break;
				case 0x88:rt_device_write(focus_uart_device, 0,Camera_Clear_Data,    6); break; //恢复初始值
				default  :break; //可能为错误命令 停止控制
		}
		*action = 0x00;
}



/**
  * @brief  Camera_Focus_Data_Analysis(变焦摄像头返回数据解析)
  * @param  控制字符数据 uint8 Data
  * @retval None
  * @notice 从第四个字节开始为控制字符
  */
void Camera_Focus_Data_Analysis(uint8 Data) //控制数据解析
{
		static uint8 RxCheck = 0;	  //尾校验字
		static uint8 RxCount = 0;	  //接收计数
		static uint8 i = 0;	   		  //

		focus_data[RxCount++] = Data;	//将收到的数据存入缓冲区中
	
	
		if(RxCount <= (OPENMV_LEN)){ //定义数据长度未包括包头和包长3个字节,+4)  
				if(focus_data[0] == 0xAA){ //接收到包头0xAA
						if(RxCount > 4){
								if(focus_data[1] == 0x55){ //接收到包头0x55
										if(RxCount >= focus_data[3]+5){ //接收到数据包长度位，开始判断什么时候开始计算校验
												for(i = 0;i <= (RxCount-2);i++){ //累加和校验
														RxCheck += focus_data[i];
												}
			
												if(RxCheck == focus_data[RxCount-1]){
														focus_data_ok = 1; //接收数据包成功
												}
												else {focus_data_ok = 0;}
												
												RxCheck = 0; //接收完成清零
												RxCount = 0;	
										}
								}
								else {focus_data_ok = 0;RxCount = 0;focus_data_ok = 0;} //接收不成功清零
						}
				}
				else {focus_data_ok = 0;RxCount = 0;focus_data_ok = 0;} //接收不成功清零
		}
		else {focus_data_ok = 0;RxCount = 0;focus_data_ok = 0;} //接收不成功清零

		
	
		if(1 == focus_data[4]){//目标为1
				target_blob_x = ((focus_data[8] <<8) | focus_data[9] );
				target_blob_y = ((focus_data[10]<<8) | focus_data[11]);
		}
		else if(2 == focus_data[4]){//当前为2
				persent_blob_x = ((focus_data[8] <<8) | focus_data[9] );
				persent_blob_y = ((focus_data[10]<<8) | focus_data[11]);		
		}

}

int get_target_x(void) //0为绿,1为红
{
		return target_blob_x;
}


int get_target_y(void)
{
		return target_blob_y;
}

int get_persent_x(void) //0为绿,1为红
{
		return persent_blob_x;
}


int get_persent_y(void)
{
		return persent_blob_y;
}






