/*
 * HMI.c
 *
 *  Created on: 2019年8月7日
 *      Author: zengwangfa
 *      Notes:  串口屏通信
 */

#include "HMI.h"
#include "DataType.h"
#include <rtthread.h>
#include "uart.h"
#include "DataProcess.h"
/*---------------------- Constant / Macro Definitions -----------------------*/

#define HMI_LEN 5

/*----------------------- Variable Declarations -----------------------------*/

uint8 hmi_data_ok = 0;
int HMI_Status_Flag = 0;//串口屏 设定状态标志位 【调试 1】or【工作2】
int HMI_Page_Number = 0;//串口屏发送的校准  纸张数

uint8 him_uart_cmd[11] = {0x66,0x64,0x2E,0x76,0x61,0x6C,0x3D,0x31,0xff,0xff,0xff};    // 01 写入成功  02 写入失败   03 正在写入... 04 提示hmi清屏
uint8 him_ret_status = 0;
uint8 hmi_data[10] = {0};
float FDC2214_Page_Data_Top[100]    = {0};
float FDC2214_Page_Data_Bottom[100] = {0};

/*----------------------- Function Implement --------------------------------*/

/* 发送给串口屏 写入的状态
01:写入成功
02:写入失败
03:正在写入
04:提示hmi清屏
*/
void Uart_Send_HMI(uint8 *cmd)
{

		him_uart_cmd[7] = *cmd; //命令对应写入
		rt_device_write(focus_uart_device, 0,him_uart_cmd	, 11);
		*cmd = 0; //命令清零
}

void FDC2214_Data_Adjust(void)//数据校准 存储
{
		him_ret_status = 3;
		Uart_Send_HMI(&him_ret_status);//正在写入
		rt_thread_mdelay(1000);
		FDC2214_Page_Data_Top   [HMI_Page_Number] = get_top_capacity();   //顶板 对应页 电容值保存
		FDC2214_Page_Data_Bottom[HMI_Page_Number] = get_bottom_capacity();//底板 对应页 电容值保存
		rt_thread_mdelay(1000);
		him_ret_status = 1;
		//Uart_Send_HMI(&him_ret_status);//正在写入


}




uint8 RxCheck = 0;	  //尾校验字
uint8 RxCount = 0;	  //接收计数

/**
  * @brief  HMI_Data_Analysis(串口屏返回数据解析)
  * @param  控制字符数据 uint8 Data
  * @retval None
  * @notice 从第四个字节开始为数据字节
  */
void HMI_Data_Analysis(uint8 Data) //控制数据解析
{

		static uint8 i = 0;	   		  //

		hmi_data[RxCount++] = Data;	//将收到的数据存入缓冲区中
	
	
		if(RxCount <= (HMI_LEN+4)){ //定义数据长度未包括包头和包长3个字节,+4)  
				if(hmi_data[0] == 0xAA){ //接收到包头0xAA
						if(RxCount > 3){
								if(hmi_data[1] == 0x55){ //接收到包头0x55
										if(RxCount >= hmi_data[2]+4){ //接收到数据包长度位，开始判断什么时候开始计算校验
												for(i = 0;i <= (RxCount-2);i++){ //累加和校验
														RxCheck += hmi_data[i];
												}
			
												if(RxCheck == hmi_data[RxCount-1]){
														hmi_data_ok = 1; //接收数据包成功
												}
												else {hmi_data_ok = 0;}
												
												RxCheck = 0; //接收完成清零
												RxCount = 0;	
										}
								}
								else {hmi_data_ok = 0;RxCount = 0;hmi_data_ok = 0;} //接收不成功清零
						}
				}
				else {hmi_data_ok = 0;RxCount = 0;hmi_data_ok = 0;} //接收不成功清零
		}
		else {hmi_data_ok = 0;RxCount = 0;hmi_data_ok = 0;} //接收不成功清零

		
		if(1 == hmi_data_ok){
				HMI_Status_Flag = hmi_data[3];//获取 工作模式位
				//rt_kprintf("HMI_Status_Flag:%d\n",HMI_Status_Flag);
				HMI_Page_Number = hmi_data[4];//获取 校准页数
				
		}
		else{
		
		
		}
		hmi_data_ok = 0;
}


uint32 get_set_page_number(void)
{
		return HMI_Page_Number;
}

uint32 get_set_status(void)
{
		return HMI_Status_Flag;
}





