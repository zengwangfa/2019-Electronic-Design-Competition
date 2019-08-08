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
/*---------------------- Constant / Macro Definitions -----------------------*/

//FLASH起始地址   W25Q128 16M 的容量分为 256 个块（Block）
#define Nor_FLASH_ADDRESS    (0x0000) 	//W25Q128 FLASH的 普通起始地址   【第个扇区】

#define IMP_FLASH_ADDRESS    (0x1000) 	//W25Q128 FLASH的 重要参数起始地址 【第个扇区】

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
		for(i = 0;i < 50;i++ ){
				ef_port_read(Nor_FLASH_ADDRESS+4*i,(Normal_Parameter+i),4);		 //Flash读取
		}
		
		for(int i = 0;i < 50;i++){
				Parameter_SelfCheck( (uint32 *)&FDC2214_Data_In_Flash[i],&Normal_Parameter[i] );//电池容量参数 3s/4s/6s
		}
		log_i("Flash_Read()");
		log_i("                      ----------");
		log_i("debug_tool            |%s     |",debug_tool_name[debug_tool]);
		log_i("                      ----------");
		return 0;
}




/* FLASH 更新 普通参数 */
void Flash_Update(void)
{
		ef_port_erase(Nor_FLASH_ADDRESS,4);	//【普通参数FLASH】先擦后写  擦除的为一个扇区4096 Byte 

		for(int i = 0;i < 50;i++){
				ef_port_write(Nor_FLASH_ADDRESS + 4*(i) ,(uint32 *)&FDC2214_Data_In_Flash[i],4); //电池容量参数 3s/4s/6s
		}
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
		

    log_i("----------------------   ---------");
		for(int i = 0;i < 50; i++){
				log_i("NO.%d : %f               %s",i,FDC2214_Data_In_Flash[i]);
		}
    log_i("----------------------   ---------");
		
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



