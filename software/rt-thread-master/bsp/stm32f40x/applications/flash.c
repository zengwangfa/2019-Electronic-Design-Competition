/*
 * flash.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  Flash读写方法
 */
#define LOG_TAG       "flash"


#include <math.h>
#include <easyflash.h>
#include <rtthread.h>
#include <elog.h>

#include "oled.h"
#include "Control.h"
#include "PID.h"
#include "debug.h"
#include "flash.h"
#include "sensor.h"
#include "drv_ano.h"
#include "servo.h"
#include "propeller.h"
#include "PropellerControl.h"
#include "gyroscope.h"
#include "EasyThread.h"
#include <stdio.h>
#include "HMI.h"
#include "DataProcess.h"
/*---------------------- Constant / Macro Definitions -----------------------*/



/*----------------------- Variable Declarations -----------------------------*/

uint32 Normal_Parameter[100]={0};

/*----------------------- Function Implement --------------------------------*/



/*******************************************
* 函 数 名：Normal_Parameter_Init_With_Flash
* 功    能：读取Flash普通参数【非常重要】
* 输入参数：none
* 返 回 值：none
* 注    意：NORMAL_PARAMETER_TABLE 枚举表 中添加自身需要的的参数的 枚举值
						NORMAL_PARAMETER_TABLE 枚举表 中添加自身需要的的参数

********************************************/
int Normal_Parameter_Init_With_Flash(void)
{
		uint8 i = 0;
		for(i = 0;i < 100 ;i++ ){
				ef_port_read(Nor_FLASH_ADDRESS+4*i,(Normal_Parameter+i),4);		 //Flash读取
		}
		ef_port_read(Nor_FLASH_ADDRESS+4*100,(uint32 *)&KT_Board_Value_In_Flash,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*101,(uint32 *)&Fiber_Board_Value_In_Flash,4);		 //Flash读取

		ef_port_read(Nor_FLASH_ADDRESS+4*102,(uint32 *)&Money.RMB_100,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*103,(uint32 *)&Money.RMB_50,4);		 //Flash读取

		ef_port_read(Nor_FLASH_ADDRESS+4*104,(uint32 *)&Div_Parameter.Div_30_40,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*105,(uint32 *)&Div_Parameter.Div_40_50,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*106,(uint32 *)&Div_Parameter.Div_50_60,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*107,(uint32 *)&Div_Parameter.Div_60_70,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*108,(uint32 *)&Div_Parameter.Div_70_80,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*109,(uint32 *)&Div_Parameter.Div_80_90,4);		 //Flash读取
		
		ef_port_read(Nor_FLASH_ADDRESS+4*110,(uint32 *)&Level,4);		 //等级
		
		ef_port_read(Nor_FLASH_ADDRESS+4*111,(uint32 *)&Money.RMB_10,4);		 //Flash读取
		ef_port_read(Nor_FLASH_ADDRESS+4*112,(uint32 *)&Money.RMB_5,4);		 //Flash读取
		
		ef_port_read(Nor_FLASH_ADDRESS+4*113,(uint32 *)&Div_Parameter.Div_90_100,4);		 //Flash读取		
		for(int i = 0;i < 100 ;i++){
				Parameter_SelfCheck( (uint32 *)&FDC2214_Data_In_Flash[i],&Normal_Parameter[i] );//电池容量参数 3s/4s/6s
		}
		log_i("Flash_Read()");
		log_w("debug_tool            |%s     |",debug_tool_name[debug_tool]);
		return 0;
}


/* FLASH 更新 普通参数 */
void Flash_Update(void)
{
		ef_port_erase(Nor_FLASH_ADDRESS,4);	//【普通参数FLASH】先擦后写  擦除的为一个扇区4096 Byte 

		for(int i = 0;i <100;i++){
				ef_port_write(Nor_FLASH_ADDRESS + 4*(i) ,(uint32 *)&FDC2214_Data_In_Flash[i],4); //
		}
		ef_port_write(Nor_FLASH_ADDRESS + 4*(100) ,(uint32 *)&KT_Board_Value_In_Flash,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(101) ,(uint32 *)&Fiber_Board_Value_In_Flash,4); 
	
		ef_port_write(Nor_FLASH_ADDRESS + 4*(102) ,(uint32 *)&Money.RMB_100,4);
		ef_port_write(Nor_FLASH_ADDRESS + 4*(103) ,(uint32 *)&Money.RMB_50,4); 
		
		ef_port_write(Nor_FLASH_ADDRESS + 4*(104) ,(uint32 *)&Div_Parameter.Div_30_40,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(105) ,(uint32 *)&Div_Parameter.Div_40_50,4);
		ef_port_write(Nor_FLASH_ADDRESS + 4*(106) ,(uint32 *)&Div_Parameter.Div_50_60,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(107) ,(uint32 *)&Div_Parameter.Div_60_70,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(108) ,(uint32 *)&Div_Parameter.Div_70_80,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(109) ,(uint32 *)&Div_Parameter.Div_80_90,4); 
		
		ef_port_write(Nor_FLASH_ADDRESS + 4*(110) ,(uint32 *)&Level,4); 
		
		ef_port_write(Nor_FLASH_ADDRESS + 4*(111) ,(uint32 *)&Money.RMB_10,4);
		ef_port_write(Nor_FLASH_ADDRESS + 4*(112) ,(uint32 *)&Money.RMB_5,4); 
		ef_port_write(Nor_FLASH_ADDRESS + 4*(113) ,(uint32 *)&Div_Parameter.Div_90_100,4); 			
}	


/* FLASH 更新 普通参数 */
//void Capacity_Flash_Update(float array[],uint8 number)
//{
//		float temp = 0.0f;
//		char str[20];
//	
//		sprintf(str,"[flash] NO.%d:%f\n",number,array[number]);
//		rt_kprintf(str);

//	
//		
//		ef_port_read( Nor_FLASH_ADDRESS+4*BATTERY_CAPACITY_e + 4*(number+1) ,(uint32 *)&temp,4);		 //Flash读取
//	
//		sprintf(str,"[flash] NO.%d:%f\n",number,temp);
//		rt_kprintf(str);
//}	


/* list 相关重要参数 */
void list_value(void)
{	
		
		static char str[30] = {0};
    log_i("---------------------- ");
		for(int i = 0;i < 50; i++){
				sprintf(str,"NO.%d : %f ",i,FDC2214_Data_In_Flash[i]);
				log_i(str);
		}
    log_i("----------------------");
		
    rt_kprintf("\n");
}
MSH_CMD_EXPORT(list_value,list some important values);





void Parameter_SelfCheck(uint32 *RealParameter,uint32 *TempParameter)
{
		//int isnan(x)函数 当x时nan返回1，其它返回0
		if( isnan(*TempParameter) == 0 ) {//如果不是无效数字 not a number,则判定为正确
				*RealParameter = *TempParameter; //Flash 数据正确则替换为真实变量
		}
		else{
				*RealParameter = 0;
		}
}



