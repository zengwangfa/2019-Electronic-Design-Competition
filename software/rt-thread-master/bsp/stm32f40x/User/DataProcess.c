/*
 * DataProcess.c
 *
 *  Created on: 2019年8月7日
 *      Author: zengwangfa
 *      Notes:  FDC2214数据处理
 */


#include "DataProcess.h"
#include "FDC2214.h"
#include <math.h>
#include <rtthread.h>
#include "filter.h"
#include "led.h"
#include "HMI.h"
#include <string.h>
#include "ioDevices.h"
unsigned int CH4_DATA;//通道值

float ShortValue4;//短路值

float Cap_Division[51]= {0};/**/

float Cap_Value[50] ={0}; //存放 10次采集的电容值

int Cap_Probability[50] ={0}; //存放可能性

PaperCountEngine_Type Paper = {
			.Finish_Flag = 2
};/*Paper数据结构体*/



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
		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
	
		if(ON_OFF == 0){/* 初始化 获取短路值*/
				ON_OFF = 1;//自锁开关
				ShortValue4 = Paper.Capacitance;
		}		
		
		if(1 == HMI_Work_Button){

				Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
				HMI_Work_Button = 0;
		}

		
		/*-----------------------短路检测、如短路直接return-------------------------*/
		if(1 == Short_Circuit_Detection()){//判定短路则不进行数据比对
				uart_send_hmi_paper_numer(Paper.PaperNumber);
				return;
		}
		for(int i = 0;i < 10;i++){
				Cap_Value[i] = get_single_capacity();//获取50组数据
		}
    rt_enter_critical();
		Paper.PaperNumber = ProbablityCapacitance(Cap_Value);	//比较数据，获取最终 纸张数
    rt_exit_critical();		
		/*-----------------------获取50组数据、解算落区间-------------------------*/	


		if(0 == HMI_Work_Button){//当按键没有按下，一直读取数据
				
		}
			
		uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据
}






/* 【校准时】获取单极板 容值 */
float get_single_capacity(void)
{
		static unsigned int res_CH4_DATA = 0;
		static float res_ch4 = 0.0f;
		static float res_ch4_arr[10] = {0.0f};
		FDC2214_GetChannelData(FDC2214_Channel_3, &res_CH4_DATA);
		res_ch4 = Cap_Calculate(&res_CH4_DATA);
//		for(uint8 i = 0;i < 10;i++){
//				FDC2214_GetChannelData(FDC2214_Channel_3, &res_CH4_DATA);
//			
//				res_ch4_arr[i] = Cap_Calculate(&res_CH4_DATA);
//		}
		
		//res_ch4 = Bubble_Filter_Float(res_ch4_arr);
		return res_ch4;
}



/* 短路判断 */
int Short_Circuit_Detection(void)
{
    //当数据在 短路数值范围变化
		//或者当值非常小的时候，判定为受到干扰
		if( (is_in_range(Paper.Capacitance,FDC2214_Data_In_Flash[0],50.0f)) ){//或者当值非常小的时候，判定为受到干扰
				Paper.ShortStatus = 1;//判定短路
				Paper.PaperNumber = 0; //如果短路即为0
				Bling_Set(&Light_Blue,100,50,0.5,0,79,0);//蓝灯提示短路
		}			
		else{
				Bling_Set(&Light_Green,100,50,0.5,0,78,0);//绿灯提示不短路
				Paper.ShortStatus = 2; //判定不短路
		}
		uart_send_hmi_is_short();
		return Paper.ShortStatus;
}		

float CapacitanceDP= 0;
/*
Cap_Division 分割
arrey 传入的原始数组
Number 数量
*/
void DataSubsection(float Cap_Division[],float arrey[],int Number)
{

		static int rec = 1;
		
		for(int i = 2;i < Number;i++){
				CapacitanceDP = (arrey[i-1]-arrey[i]) /2.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;

		}
		if(rec==1){
				Cap_Division[0] =arrey[1]+(arrey[1]-arrey[2]) /2.0f;
				rec = 0;
		}
}

int Probability_Max = 0;
/*
CompareArrey 
*/
uint8 ProbablityCapacitance(float CompareArrey[])	//传入 需要比较的数据
{

		memset(Cap_Probability,0,sizeof(Cap_Probability));//清空电容值落点可能性
		for(int i=0;i<=50;i++ ){
				for(int j=0; j<10 ;j++){
						if( (CompareArrey[j] < Cap_Division[i])  && (CompareArrey[j] >= Cap_Division[i+1])){
								Cap_Probability[i]++;
						}
				}
		}
		for(int n = 0;n < 49;n++){
				if(Cap_Probability[n] > Cap_Probability[Probability_Max]){
						Probability_Max = (n + 1);
				}
				if(Cap_Probability[0] >= 25){	//1纸张的特殊处理
						Probability_Max = 1;
				}	
		}
		return Probability_Max;
}

