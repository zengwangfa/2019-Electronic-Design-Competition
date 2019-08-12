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
#include <stdio.h>
#include "uart.h"
#include "DataProcess.h"
#include "flash.h"
#include <string.h>
#include "ioDevices.h"
#include <easyflash.h>
#include "flash.h"
#include "my2490.h"

/*---------------------- Constant / Macro Definitions -----------------------*/

#define HMI_LEN 5

/*----------------------- Variable Declarations -----------------------------*/

uint8 hmi_data_ok = 0;
int HMI_Status_Flag = 0;//串口屏 设定状态标志位 【调试 1】or【工作2】
int HMI_Data = 0;
int HMI_Page_Number = 0;//串口屏发送的校准  纸张数

int HMI_Debug_Write_Button = 0; //写入数据Flag
int HMI_Work_Button = 0; /* 检测 确认 按钮*/
int Material_Button = 0; /* 材料 检测 确认按钮*/
int Material_Debug_Write_Button = 0; /* 材料 写入数据*/

int Money_Button = 0; /* 纸币 检测 确认按钮*/
int Money_Debug_Write_Button = 0; /* 纸币  写入数据*/
//76 61 30 2E 76 61 6C 3D 31 ff ff ff
//76 61 30 2E 76 61 6C 3D 31 ff ff ff   va0.val=1
uint8 him_uart_cmd[12] = {0x76,0x61,0x30,0x2E,0x76,0x61,0x6C,0x3D,0x31,0xFF,0xFF,0xFF};    // 01 写入成功  02 写入失败   03 正在写入... 04 提示hmi清屏
//76 61 31 2E 76 61 6C 3D 31 30 30 ff ff ff
//76 61 31 2E 76 61 6C 3D 31 30 30 ff ff ff  va1.val=100
uint8 him_uart_nmber_cmd[14] = {0x76,0x61,0x31,0x2E,0x76,0x61,0x6C,0x3D,0x31,0x30,0x30,0xff,0xff,0xff};

//76 61 32 2E 76 61 6C 3D 31 ff ff ff   va2.val=1
uint8 him_uart_short_cmd[12] = {0x76,0x61,0x32,0x2E,0x76,0x61,0x6C,0x3D,0x30,0xff,0xff,0xff};
// rest (HMI复位指令)72 65 73 74 ff ff ff
uint8 him_uart_reboot_cmd[7] = {0x72,0x65,0x73,0x74,0xFF,0xFF,0xFF};

uint8 him_uart_material_cmd[12] = {0x76,0x61,0x33,0x2E,0x76,0x61,0x6C,0x3D,0x30,0xff,0xff,0xff};

uint8 him_uart_money_cmd[12] = {0x76,0x61,0x34,0x2E,0x76,0x61,0x6C,0x3D,0x30,0xff,0xff,0xff};

uint8 him_ret_status = 0;
uint8 hmi_data[10] = {0};

float FDC2214_Paper_Data[100]  = {0.0f};
float FDC2214_Data_In_Flash[100] = {0.0f};//写入

/*----------------------- Function Implement --------------------------------*/

void uart_send_hmi_reboot(void)
{
		rt_device_write(focus_uart_device, 0,him_uart_reboot_cmd	, 7);
}

/* 发送给串口屏 写入的状态
01:写入成功
02:写入失败
03:正在写入
04:提示hmi清屏
*/
void uart_send_hmi_writer_status(uint8 *cmd)//发送给 hmi写入的状态
{

		him_uart_cmd[8] = *cmd + 0x30; //命令+0x30  转成对应的ASCII 对应写入
		rt_device_write(focus_uart_device, 0,him_uart_cmd	, sizeof(him_uart_cmd));

}



void uart_send_hmi_paper_numer(uint8 N_number)  //发送给hmi 纸张数量
{ 	

		him_uart_nmber_cmd[8]  = (N_number/100%10) + 0x30;  //百位
		him_uart_nmber_cmd[9]  = (N_number/10%10) + 0x30; //十位
		him_uart_nmber_cmd[10] = (N_number/1%10) + 0x30;//个位
	
		rt_device_write(focus_uart_device, 0,him_uart_nmber_cmd	, sizeof(him_uart_nmber_cmd));	

}


void uart_send_hmi_adjust_data(uint8 N_number)  //发送给hmi 前5个校准数据
{ 	
		static uint8 end[3] = {0xFF,0xFF,0xFF};
    static char adjust_str[50];
    static char adjust_str_len = 0;
		static uint8 number = 0;
		number = HMI_Page_Number % 5;
		if(HMI_Page_Number !=0 && 0 == number){
				number = 5;
		}
		sprintf(adjust_str,"t%d.txt=\"%d: %.4f\"",number,HMI_Page_Number,FDC2214_Data_In_Flash[HMI_Page_Number]);//发送给串口屏 t1.txt="xxxx"
		
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);	

}


void uart_send_hmi_30_90_flash_data(void)  //发送给hmi 前5个校准数据
{ 	
		static uint8 end[3] = {0xFF,0xFF,0xFF};
		char adjust_str[35];
		char adjust_str_len = 0;
		
		sprintf(adjust_str,"n1.val=%d",Div_Parameter.Div_30_40);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);	
		
		sprintf(adjust_str,"n2.val=%d",Div_Parameter.Div_40_50);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);			

		sprintf(adjust_str,"n3.val=%d",Div_Parameter.Div_50_60);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);			
		
		sprintf(adjust_str,"n4.val=%d",Div_Parameter.Div_60_70);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);		
		
		sprintf(adjust_str,"n5.val=%d",Div_Parameter.Div_70_80);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);		
		
		sprintf(adjust_str,"n6.val=%d",Div_Parameter.Div_80_90);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);		

}


char adjust_str[35];
char adjust_str_len = 0;
void uart_send_hmi_now_level(void)  //发送给hmi 当前等级
{ 	
		static uint8 end[3] = {0xFF,0xFF,0xFF};

		
		sprintf(adjust_str,"n0.val=%d",Level);//发送给串口屏 t1.txt="xxxx"
		adjust_str_len =  strlen(adjust_str);
		rt_device_write(focus_uart_device, 0,adjust_str	,adjust_str_len);	
		rt_device_write(focus_uart_device, 0,end	, 3);	
		
}


void uart_send_hmi_is_short(void)  //发送给hmi 是否短路
{ 	

		him_uart_short_cmd[8] = 0x30 + Paper.ShortStatus;
		
		rt_device_write(focus_uart_device, 0,him_uart_short_cmd	, sizeof(him_uart_short_cmd));//向HMI发送短路信息

}

void uart_send_hmi_is_material(uint8 material)  //发送给hmi 是什么材料
{ 	
		him_uart_material_cmd[8] = 0x30 + material;				
		
		rt_device_write(focus_uart_device, 0,him_uart_material_cmd	, sizeof(him_uart_material_cmd));//向HMI发送短路信息
}

void uart_send_hmi_is_money(uint8 money)  //发送给hmi 是什么材料
{ 	
		him_uart_money_cmd[8] = 0x30 + money;				
		
		rt_device_write(focus_uart_device, 0,him_uart_money_cmd	, sizeof(him_uart_money_cmd));//向HMI发送短路信息
}


float Data_Diff_Value = 0.0f; //FLash 上下两次的差值
uint32 _cnt = 0;
/* 发送给串口屏 写入的状态
01:写入成功
02:写入失败
03:正在写入
04:提示hmi清屏
*/
void FDC2214_Data_Adjust(void)//数据校准 存储
{
		static char str[30] = {0};

		_cnt ++;
		FDC2214_Paper_Data[HMI_Page_Number] = get_single_capacity(); /* 获取电容 数值*/

		if(HMI_Page_Number > 1 && 0x01 == HMI_Debug_Write_Button){
				Data_Diff_Value = FDC2214_Paper_Data[HMI_Page_Number] - FDC2214_Paper_Data[HMI_Page_Number-1];
		}
		if(0x01 == HMI_Debug_Write_Button){//只有按下才写入
				FDC2214_Data_In_Flash[HMI_Page_Number] = FDC2214_Paper_Data[HMI_Page_Number] ;//单板 对应页 电容值保存
				Flash_Update();/* FLASH 写入 */
				sprintf(str,"pagenum:%d,cap:%f\n",HMI_Page_Number,FDC2214_Data_In_Flash[HMI_Page_Number]);
				rt_kprintf(str);
				Paper.Status = 0x01; //写入成功
				uart_send_hmi_writer_status(&Paper.Status);//返回状态信息
				HMI_Debug_Write_Button = 0; //写入状态清零

		}
		else if(0x00 == HMI_Debug_Write_Button && _cnt >= 200){
				_cnt = 0;
				Paper.Status = 0x04; //清空	
				uart_send_hmi_writer_status(&Paper.Status);//返回状态信息	
		}
		Short_Circuit_Detection();  //短路检测
		if(HMI_Page_Number != 0){//不等于0 发送
				uart_send_hmi_adjust_data(HMI_Page_Number);
		}
}


uint8 last_level = 0;
/**
  * @brief  HMI_Data_Analysis(串口屏返回数据解析)
  * @param  控制字符数据 uint8 Data
  * @retval None
  * @notice 从第四个字节开始为数据字节
  */
void HMI_Data_Analysis(uint8 Data) //控制数据解析
{

		static uint8 i = 0;	   		  //
		static uint8 RxCheck = 0;	  //尾校验字
		static uint8 RxCount = 0;	  //接收计数

	
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
				HMI_Status_Flag = hmi_data[3];//获取 工作模式位(页面s)
				HMI_Data = hmi_data[4];
				switch(HMI_Status_Flag){
				
						case 0x01:
											HMI_Debug_Write_Button = 0; 	//【校准页面】只是单单进入页面
											break;	
						case 0x02:
											HMI_Page_Number = HMI_Data;//【校准页面】 获取 指定的校准页数
											HMI_Debug_Write_Button = 1; 	//写入Flash
											break;				
						case 0x03:
											if(0x01 == HMI_Data){
													HMI_Work_Button = 1;         	//【工作页面】 按下锁定按钮
											}
											else if(0x02 == HMI_Data){
													HMI_Work_Button = 2;         	//【工作页面】 清除锁定								
											}
											break;				
						case 0x04:
											break;//扩展功能页面				
						case 0x05:
											break;//打印机页面


					 case 0x06:
										if( 0x01 == HMI_Data){
												Material_Button = 1; 
										}
										else if(0x02 == HMI_Data){
												Material_Debug_Write_Button = 1;
										}
										else if(0x03 == HMI_Data){
												Material_Debug_Write_Button = 2;
										}
										else{
												Material_Button = 0; //清除锁定
										}
										break;	
					 case 0x07:
										if( 0x01 == HMI_Data){
												Money_Button = 1; //清除锁定
										}
										else if(0x02 == HMI_Data){
												Money_Debug_Write_Button = 1;
										}
										else if(0x03 == HMI_Data){
												Money_Debug_Write_Button = 2;
										}
										else if(0x04 == HMI_Data){
												Money_Debug_Write_Button = 3;
										}
										else if(0x05 == HMI_Data){
												Money_Debug_Write_Button = 4;
										}
										else{
												Money_Button = 0; //清除锁定
										}
					 case 0x08:
										uart_send_hmi_30_90_flash_data();//进入调参界面,发送存入Flash的值
										break;
					 case 0x09:
										Div_Parameter.Div_30_40 = HMI_Data;//获取数值
										Flash_Update();
										break;	
					 case 0x0A:
										Div_Parameter.Div_40_50 = HMI_Data;//获取数值
										Flash_Update();

										break;		
					 case 0x0B:
										Div_Parameter.Div_50_60 = HMI_Data;//获取数值
										Flash_Update();
										break; 
					 case 0x0C:
										Div_Parameter.Div_60_70 = HMI_Data;//获取数值
										Flash_Update();
										break;  
					 case 0x0D:
										Div_Parameter.Div_70_80 = HMI_Data;//获取数值
										Flash_Update();
										break;  
					 case 0x0E:
										Div_Parameter.Div_80_90 = HMI_Data;//获取数值
										Flash_Update();
										break;  
					 
					 
					 
					 
					 case 0xFE:
										uart_send_hmi_now_level();//进入等级页面
										rt_thread_mdelay(100);
										uart_send_hmi_now_level();//进入等级页面
										break;  
					 case 0xFF:
										uart_send_my2490_now_sounds();//如果大于，发送
								
										Level = HMI_Data;//获取等级
										Flash_Update();
										break;  
				}
				
			

		}
		hmi_data_ok = 0;
}







