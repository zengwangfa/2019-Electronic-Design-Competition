/*
 * oled.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  OLED状态、参数指示
 */
#define LOG_TAG    "oled"

#include "oled.h"
#include <rtthread.h>
#include <math.h>
#include <elog.h>
#include <stdio.h>
#include "sys.h"

#include "led.h"
#include "rc_data.h"
#include "drv_cpu_temp.h"
#include "drv_cpuusage.h"
#include "drv_oled.h"
#include "drv_adc.h"
#include "sensor.h"

#include "ioDevices.h"

#include "gyroscope.h"
#include "sensor.h"
#include "filter.h"

#include "EasyThread.h"
#include "DataProcess.h"
#include "HMI.h"

/* 自定义OLED 坐标系如下: 

	127 ↑y
			---------
			|	      |
			|				|
			|				|
			|				|
			|				|
			|				|
		(0,0)-----→x  
							63
*/
/*---------------------- Static / Constant / Macro Definitions ---------------------*/


/*--------------------------- Variable Declarations --------------------------------*/

volatile MENU_LIST_Enum MENU = DebugPage;//OLED初始页面为 状态页. volatile是一种类型修饰符。
																				  //volatile 的作用 是作为指令关键字，确保本条指令不会因编译器的优化而省略，且要求每次直接在其内存中读值。

/* OLED 变量 初始化. */
Oled_Type oled = {	 
									 .pagenum = SwitchPage,		 //页码 pagenum
									 .pagechange = SwitchPage, //暂存页码 检测页码是否改变 pagechange
									 .pagechange_flag = 0,     //页码改变标志位 pagechange flag
									 .pagename = //页名定义 pagename
										{	
												"SwitchPage",
												"DebugPage",
												"WorkPage",
												"LockPage"} 							
};
/*----------------------- Function Implement --------------------------------*/

/* OLED 线程初始化 */
int oled_thread_init(void)
{
    rt_thread_t oled_tid;
		/*创建动态线程*/
    oled_tid = rt_thread_create("oled", //线程名称
                    oled_thread_entry,	//线程入口函数【entry】
                    RT_NULL,				    //线程入口函数参数【parameter】
                    2048,							  //线程栈大小，单位是字节【byte】
                    10,								  //线程优先级【priority】
                    10);							  //线程的时间片大小【tick】= 100ms

    if (oled_tid != RT_NULL){
				OLED_Init();
				log_i("OLED_Init()");
				rt_thread_startup(oled_tid);

				oled.pagechange = oled.pagenum;  //初始化暂存页面
		}
		return 0;
}
INIT_APP_EXPORT(oled_thread_init);

/*******************************************
* 函 数 名：menu_define
* 功    能：菜单选择
* 输入参数：none
* 返 回 值：none
* 注    意：页码切换时，蜂鸣器响一声
********************************************/
void menu_define(void) //菜单定义
{
	  if(oled.pagenum >= OLED_Page_MAX || oled.pagenum <= SwitchPage) oled.pagenum = SwitchPage; //超出页面范围 则为第一页
		if(oled.pagechange != oled.pagenum){ //当页码改变
				OLED_Clear(); //清屏
				//Buzzer_Set(&Beep,1,1);
				//rt_kprintf("Current Menu_Page: %s \n",oled.pagename[oled.pagenum-1]);
				oled.pagechange_flag = 1;
		}
		else {oled.pagechange_flag = 0;}
		oled.pagechange = oled.pagenum;
		
		oled.pagenum = HMI_Status_Flag;//获取状态

		switch(oled.pagenum){
				case 0:{
						MENU = SwitchPage;OLED_SwitchPage();		break;
				}
				case 1:{
						MENU = DebugPage;OLED_DebugPage();		break;
				}
				case 2:{
						MENU = WorkPage;OLED_WorkPage();break;
				}
				case 3:{
						MENU = FuncPage;OLED_FuncSwitchPage();break;
				}
				case 4:{
						MENU = PrintPage;OLED_PrintPage();break;
				}
				case 5:{
						MENU = MaterPage;OLED_MaterPage();break;
				}
				default:OLED_SwitchPage();	break;
		}
}

void OLED_SwitchPage(void)
{
		OLED_ChineseString(32,0,31,34,16);//选择菜单
		
		OLED_ChineseString(0,32,35,36,16);//调试模式
		OLED_ChineseString(0,48,37,38,16);
		
		OLED_ChineseString(48,32,41,42,16);//调试模式
		OLED_ChineseString(48,48,43,44,16);
		
		OLED_ChineseString(96,32,39,40,16);//工作模式
		OLED_ChineseString(96,48,37,38,16);
	  OLED_Refresh_Gram();//更新显示到OLED
}



float OperationTime = 0;
/*******************************************
* 函 数 名：OLED_DebugPage
* 功    能：
* 输入参数：none
* 返 回 值：none
* 注    意：校准页面
********************************************/
void OLED_DebugPage(void)
{
		static char str[50] = {0};
	
		OLED_ChineseString(0,0,2,3,16);  		//打印当前电容
		sprintf(str,": %.5f     ",FDC2214_Paper_Data[HMI_Page_Number]);
		OLED_ShowString(32,0, (uint8 *)str,16);

		OLED_ChineseString(0,16,0,1,16);
		OLED_ChineseString(32,16,4,4,16);
		OLED_ChineseString(48,16,15,15,16);   //打印当前页数
		sprintf(str,": %d   ",HMI_Page_Number);		  //打印当前页数
		OLED_ShowString(72,16, (uint8 *)str,16);

		OLED_ChineseString(0,48,13,14,16);  		// 状态
		OLED_ShowString(32,48, (uint8 *)":",16); 		

		if(1 == Paper.ShortStatus){//当短路
				OLED_ChineseString(40,48,11,12,16);
				OLED_ShowString(72,48, (uint8 *)"    ",16);
		}
		else{
				OLED_ChineseString(40,48,10,10,16);
				OLED_ChineseString(56,48,11,12,16);
		}
	
		OLED_Refresh_Gram();//更新显示到OLED						
}

/*******************************************
* 函 数 名：OLED_GyroscopePage
* 功    能：显示九轴模块参数【加速度、角速度、欧拉角、磁场】
* 输入参数：none
* 返 回 值：none
* 注    意：OLED第二页 【九轴参数页】
********************************************/
void OLED_WorkPage(void)
{
		static char str[50] = {0};
		
		switch(Paper.Status){
				case 0:
						OLED_ChineseString(40,0,10,10,16);
						OLED_ChineseString(56,0,16,17,16);				//纸张测数器未开始的状态	
						break;
				case 1:
						OLED_ChineseString(40,0,19,22,16);				//纸张测数器完成的状态
						break;
				case 2:
						OLED_ChineseString(40,0,16,18,16); 				//纸张测数器运行的状态
						break;		
		}
			
		OLED_ChineseString(0,0,13,14,16);  		// 状态
		OLED_ShowString(32,0, (uint8 *)":",16); 
		
		OLED_ChineseString(0,16,2,3,16);  		//打印当前电容
		sprintf(str,": %.3f",Paper.Capacitance);
		OLED_ShowString(32,16, (uint8 *)str,16);

		OLED_ChineseString(0,32,23,26,16);   	//打印当前张数
		OLED_ShowString(64,32, (uint8 *)":",16); 
		
		if(1 == Paper.Finish_Flag )	{			//读取纸张数量完成
		
				sprintf(str,"%3d",Paper.PaperNumber);  //测试纸张页数
				OLED_ShowString(72,32, (uint8 *)str,16); 
				Paper.Status = 1;									//将状态至于完成
				Paper.Finish_Flag = 0; //锁定纸张显示
		}
		else if(2 == Paper.Finish_Flag){
				sprintf(str,"%3d",Paper.PaperNumber);  //测试纸张页数
				OLED_ShowString(72,32, (uint8 *)str,16); 
		
		}
		
		OLED_ChineseString(0,48,27,30,16);	  	//打印使用时间
		sprintf(str,":%.3fs",OperationTime);
		OLED_ShowString(64,48, (uint8 *)str,16);
		
	  OLED_Refresh_Gram();//更新显示到OLED
}


/* 拓展功能选择 页面*/
void OLED_FuncSwitchPage(void)
{
		OLED_ChineseString(32,0,41,44,16);

		OLED_ChineseString(8,32,45,47,16);		//打印机纸盒检测
		OLED_ChineseString(0,48,48,51,16);
		
		OLED_ChineseString(96,32,52,53,16);		//材料识别
		OLED_ChineseString(96,48,54,55,16);
		
		OLED_Refresh_Gram();//更新显示到OLED			
}


/* 打印机 页面*/
void OLED_PrintPage(void)
{
		static uint8 PrinterFlag = 1;
		static char str[20] = {0};
		OLED_ChineseString(32,0,45,47,16);          	//打印机
		
		OLED_ChineseString(0,32,0,1,16);				//打印机剩余纸张
		OLED_ChineseString(32,32,56,57,16);
		OLED_ShowString(64,32,(uint8 *)":",16);
		OLED_ChineseString(108,32,24,24,16);
		sprintf(str,"%3d",Paper.PaperNumber);  //测试纸张页数
		OLED_ShowString(72,32, (uint8 *)str,16); 
		
		if(PrinterFlag == 1){
				OLED_ChineseString(32,48,58,61,16);				//缺纸警报
		}
		else {
				OLED_ChineseString(32,48,62,62,16);
		}
		
		if(Paper.PaperNumber <=5){
				PrinterFlag = 1;
		}
		else{
				PrinterFlag =0;
		}
		
		OLED_Refresh_Gram();//更新显示到OLED			
		
}


/* 材料检测 页面*/
void OLED_MaterPage(void)
{
static uint8 MaterialFlag = 0;
	
		OLED_ChineseString(32,0,52,55,16);  //材料识别
			
		OLED_ChineseString(0,32,0,1,16);				//当前材料
		OLED_ChineseString(32,32,52,53,16);
		OLED_ShowString(64,32,(uint8 *)":",16);
		if(MaterialFlag == 0)							//无
		{
				OLED_ChineseString(72,32,63,63,16);
		}
		if(MaterialFlag == 1){							//KT板
				OLED_ShowString(72,32,(uint8 *)"KT",16);
				OLED_ChineseString(72,32,66,66,16);
		}
		if(MaterialFlag == 2)	{						//纤维板
		
				OLED_ChineseString(72,32,64,66,16);
		}

	

		OLED_Refresh_Gram();//更新显示到OLED			

}

/*******************************************
* 函 数 名：Boot_Animation
* 功    能：开机动画
* 输入参数：none
* 返 回 值：none
* 注    意：none
* 说    明：绘制ETA简易LOGO
********************************************/
void Boot_Animation(void)
{
		static uint8 x=0,y=0;

		for(x = 63;x>=18;x--){
				OLED_DrawPoint(108-0.7*x,x,1);//画斜线 x,y反置
				OLED_DrawPoint(17 +0.7*x,x,1);
				y = 64-x;
				OLED_DrawPoint(64-0.7*y,y,1);
				OLED_DrawPoint(64+0.7*y,y,1);
				rt_thread_mdelay(20);
			  OLED_Refresh_Gram();//更新显示到OLED
		}
		
		for(x = 30;x <= 94;x++){
				OLED_DrawPoint(125-x,47,1);
				OLED_DrawPoint(x,18,1);
				rt_thread_mdelay(20);
			  OLED_Refresh_Gram();//更新显示到OLED
		}

		OLED_ShowString(60,20,(uint8 *)"E",16);
	  OLED_Refresh_Gram();//更新显示到OLED
		rt_thread_mdelay(1000);
		
}



/* OLED 下一页 */
void next_oled_page(void)
{
		oled.pagenum ++;
}
MSH_CMD_EXPORT(next_oled_page,next_oled_page[page++]);

/* OLED 上一页 */
void last_oled_page(void)
{
		oled.pagenum --;
}
MSH_CMD_EXPORT(last_oled_page,last_oled_page[page--]);



