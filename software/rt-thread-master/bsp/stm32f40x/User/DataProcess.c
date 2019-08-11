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
#include "my2490.h"
#include "flash.h"
#include "ioDevices.h"

unsigned int CH4_DATA;//通道值

float ShortValue4;//短路值

float Cap_Division[101]= {0};/**/

float Cap_Value[10] ={0}; //存放 10次采集的电容值

int Cap_Probability[100] ={0}; //存放可能性

PaperCountEngine_Type Paper = {
			.Finish_Flag = 2
};/*Paper数据结构体*/

float KT_Board_Value_In_Flash = 0.0f; //KT板Flash数值
float Fiber_Board_Value_In_Flash = 0.0f;//纤维板 Flash数值

float Money_100_In_Flash = 0.0f; //100元
float Money_50_In_Flash = 0.0f;  //50元

int Div_40_50_Parameter = 30; //切分 40~50张的系数
int Div_50_60_Parameter = 30; //切分 40~50张的系数
int Div_60_70_Parameter = 30; //切分 40~50张的系数
int RMB_Value = 0;
//判断data是否在 value ± range内
int is_in_range(float data,float value,float range)
{
		if( (data < (value+range) && data > (value-range))){
				return 1;
		}		
		return 0;
}

void Get_Paper(void){

		for(int i = 0;i < 10;i++){
				Cap_Value[i] = get_single_capacity();//获取50组数据
		}	
    rt_enter_critical(); /* 线程锁定 */
		Paper.PaperNumber = ProbablityCapacitance(Cap_Value);	//比较数据，获取最终 纸张数
    rt_exit_critical();		 
}


/* 电容 纸张 检测 */
void Capcity_Paper_Detection(void)
{

		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/


		
		/*-----------------------短路检测、如短路直接return-------------------------*/
		if(1 == Short_Circuit_Detection()){//判定短路则不进行数据比对
				if(1 == HMI_Work_Button){
						uart_send_my2490_now_status(my2490_number_array,Paper.PaperNumber);
						Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
						HMI_Work_Button = 0;
				}
				uart_send_hmi_paper_numer(Paper.PaperNumber);
				rt_thread_mdelay(5); //当直接返回是，延时，以防读取过快
				return;
		}

		Get_Paper();/* 获取纸张数量 */

		/*-----------------------获取50组数据、解算落区间-------------------------*/	

		if(1 == HMI_Work_Button){

				Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
				uart_send_my2490_now_status(my2490_number_array,Paper.PaperNumber);
				HMI_Work_Button = 0;
		}

		if(0 == HMI_Work_Button){//当按键没有按下，一直读取数据
				
		}
			
		uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据
}

/* 打印机 纸张检测 */
void Printer_Paper_Detection(void)
{

		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		Get_Paper();/* 获取纸张数量 */	
	
		Short_Circuit_Detection() ;
	
		if(Paper.PaperNumber < 5 ){//小于5就一直报警
				Buzzer_Set(&Beep,1,1);
		}
		rt_thread_mdelay(100);
		uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据

}


/* 材料 检测 */
void Material_Detection(void)
{
		static int is_material = 0;
	
		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		if(Material_Debug_Write_Button == 1){
				KT_Board_Value_In_Flash = Paper.Capacitance;
				Flash_Update();
		
		}
		else if(Material_Debug_Write_Button == 2){
				Fiber_Board_Value_In_Flash = Paper.Capacitance;
				Flash_Update();		
		}
		Material_Debug_Write_Button = 0;
		
		if(Material_Button == 1){//当检测按钮按下
				if( (fabs(Paper.Capacitance-KT_Board_Value_In_Flash) < fabs(Paper.Capacitance-Fiber_Board_Value_In_Flash) )  && Paper.Capacitance >= 100 ){//比较更靠近谁
						is_material = 1; //KT板
				}
				else if( (fabs(Paper.Capacitance-KT_Board_Value_In_Flash) > fabs(Paper.Capacitance-Fiber_Board_Value_In_Flash) )  && Paper.Capacitance >= 100 ) {
						is_material = 2; //纤维板
				}
				else{
					  is_material = 0; //无
				}
				Material_Button = 0;
		}
		if(Short_Circuit_Detection() == 1){
				Material_Button = 0; //无
		}
		uart_send_hmi_is_material(is_material);//1为KT板，2为纤维板

}




/* 纸币 检测 */
void Money_Detection(void)
{

	
		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		if(Money_Debug_Write_Button == 1){
				Money_100_In_Flash = Paper.Capacitance;
				Flash_Update();

		}
		else if(Money_Debug_Write_Button == 2){
				Money_50_In_Flash = Paper.Capacitance;
				Flash_Update();		
		}
		Money_Debug_Write_Button = 0;
		if(Money_Button == 1){//当检测按钮按下

				if( (fabs(Paper.Capacitance-Money_100_In_Flash) < fabs(Paper.Capacitance-Money_50_In_Flash) )  && Paper.Capacitance >= 100 )	{//比较更靠近谁
						RMB_Value = 1; //100元
				}
				else if( (fabs(Paper.Capacitance-Money_100_In_Flash) > fabs(Paper.Capacitance-Money_50_In_Flash) )  && Paper.Capacitance >= 100 ) {
						RMB_Value = 2; //50元
				}
				else{
					  RMB_Value = 0; //无
				}
				Money_Button = 0;
				uart_send_hmi_is_money(RMB_Value);//1为KT板，2为纤维板	
		}
		
		if(Short_Circuit_Detection() == 1){
				RMB_Value = 0; //无
		}
		rt_thread_mdelay(5);


}

/* 【校准时】获取单极板 容值 */
float get_single_capacity(void)
{
		static unsigned int res_CH4_DATA = 0;
		static float res_ch4 = 0.0f;
		
		FDC2214_GetChannelData(FDC2214_Channel_3, &res_CH4_DATA);
		res_ch4 = Cap_Calculate(&res_CH4_DATA);

		return res_ch4;
}



/* 短路判断 */
int Short_Circuit_Detection(void)
{
    //当数据在 短路数值范围变化
		//或者当值非常小的时候，判定为受到干扰
		if( //((is_in_range(FDC2214_Paper_Data[0],FDC2214_Data_In_Flash[0],50.0f)) && HMI_Status_Flag == 1) \
			//||((is_in_range(Paper.Capacitance,FDC2214_Data_In_Flash[0],50.0f)) && HMI_Status_Flag == 2) 
			//||((is_in_range(Paper.Capacitance,FDC2214_Data_In_Flash[0],50.0f)) && HMI_Status_Flag == 4) )
				(FDC2214_Paper_Data[0] <= 50.0f && HMI_Status_Flag == 1)  || (Paper.Capacitance <= 50.0f && HMI_Status_Flag == 2) || (Paper.Capacitance <= 50.0f && HMI_Status_Flag == 3) 
		
		)
		{
				Paper.ShortStatus = 1;//判定短路
				Paper.PaperNumber = 0; //如果短路即为0
				Bling_Set(&Light_Red,100,50,0.5,0,77,0);//红灯提示短路
		}			
		else{
				Bling_Set(&Light_Green,100,50,0.5,0,78,0);//绿灯提示不短路
				Paper.ShortStatus = 2; //判定不短路
		}
		uart_send_hmi_is_short(); //发送是否短路
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
		
		for(int i = 2;i < 30;i++){
				CapacitanceDP = (arrey[i-1]-arrey[i]) /2.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;

		}
		for(int i=30;i<40;i++){
				CapacitanceDP = 30*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=40;i<50;i++){
				CapacitanceDP = Div_40_50_Parameter*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		
		for(int i=50;i<60;i++){
				CapacitanceDP = Div_50_60_Parameter*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=60;i<72;i++){
				CapacitanceDP = Div_60_70_Parameter*(arrey[i-1]-arrey[i]) /100.0f;
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
	
		for(int i=0;i<=70;i++ ){
				for(int j=0; j<10 ;j++){
						if( (CompareArrey[j] < Cap_Division[i])  && (CompareArrey[j] >= Cap_Division[i+1])){
								Cap_Probability[i]++;
						}
				}
		}
		for(int n = 0;n < 69 ;n++){
				if(Cap_Probability[n] > Cap_Probability[Probability_Max]){
						Probability_Max = (n + 1);
				}
				if(Cap_Probability[0] >= 25){	//1纸张的特殊处理
						Probability_Max = 1;
				}	
		}
		return Probability_Max;
}

