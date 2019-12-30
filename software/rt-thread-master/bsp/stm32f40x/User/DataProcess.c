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
#include "nbiot.h"

unsigned int CH4_DATA;//通道值

float ShortValue4;//短路值

float Cap_Division[110]= {0};/**/

float Cap_Value[10] ={0}; //存放 10次采集的电容值

int Cap_Probability[110] ={0}; //存放可能性

PaperCountEngine_Type Paper = {
			.Finish_Flag = 2
};/*Paper数据结构体*/

float KT_Board_Value_In_Flash = 0.0f; //KT板Flash数值
float Fiber_Board_Value_In_Flash = 0.0f;//纤维板 Flash数值

int RMB_Value = 0;

Div_Parameter_Type Div_Parameter = {30,30,30,30,30,30,30};//区间分段的系数
Money_Type Money;

int Level = 0;//等级 0~100

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
    rt_enter_critical(); /* 调度器上锁 */
		Paper.PaperNumber = ProbablityCapacitance(Cap_Value);	//比较数据，获取最终 纸张数
    rt_exit_critical();	 /* 调度器解锁 */	 
}


/* 电容 纸张 检测 */
void Capcity_Paper_Detection(void)
{
		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/

		/*-----------------------短路检测、如短路直接return-------------------------*/
		if(0x01 == Short_Circuit_Detection()){//判定短路则不进行数据比对
				if(0x01 == HMI_Work_Button){
						uart_send_my2490_now_status(my2490_number_array,Paper.PaperNumber);
						Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
						HMI_Work_Button = 0;
						Nb_Iot_Send_data(Paper.PaperNumber,(uint16)Paper.Capacitance,Paper.ShortStatus);
				}
				
				uart_send_hmi_paper_numer(Paper.PaperNumber);
				rt_thread_mdelay(5); //当直接返回是，延时，以防读取过快
				return;
		}
		
		/*-----------------------获取50组数据、解算落区间-------------------------*/	


		if(0x01 == HMI_Work_Button){
				Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
				Get_Paper();/* 获取纸张数量 */
				Get_Paper();/* 获取纸张数量 */
				uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据
				uart_send_my2490_now_status(my2490_number_array,Paper.PaperNumber);

				rt_thread_mdelay(200);
				uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据
				HMI_Work_Button = 0;
				Nb_Iot_Send_data(Paper.PaperNumber,(uint16)Paper.Capacitance,Paper.ShortStatus);//NB-IOT发送信息
		}

		else if( (0x02 == HMI_Work_Button )) {//当标志位为2，即 实时检测
				Get_Paper();/* 获取纸张数量 */
				uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据
		}
			

}

/* 打印机 纸张检测 */
void Printer_Paper_Detection(void)
{

		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		if(Short_Circuit_Detection() != 1){
				Get_Paper();/* 获取纸张数量 */	
		}
	

	
		if(Paper.PaperNumber < 5 ){//小于5就一直报警
				Buzzer_Set(&Beep,1,1);
		}
		rt_thread_mdelay(100);
		uart_send_hmi_paper_numer(Paper.PaperNumber);	//发送数据

}

int is_material = 0;
/* 材料 检测 */
void Material_Detection(void)
{

		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		if(0x01 == Material_Debug_Write_Button){
				KT_Board_Value_In_Flash = Paper.Capacitance;
		}
		else if(0x02 == Material_Debug_Write_Button){
				Fiber_Board_Value_In_Flash = Paper.Capacitance;
		}
		if(0x00 != Material_Debug_Write_Button){
				Buzzer_Set(&Beep,1,1);		
				Flash_Update();				
		}
		Material_Debug_Write_Button = 0;
		
		if(Material_Button == 1){//当检测按钮按下
				Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
				if( is_in_range(Paper.Capacitance,KT_Board_Value_In_Flash,15) ){//比较更靠近谁
						is_material = 1; //KT板
				}
				else if( is_in_range(Paper.Capacitance,Fiber_Board_Value_In_Flash,15) ) {
						is_material = 2; //纤维板
				}
				else{
					  is_material = 0; //无
				}
				Material_Button = 0;
		}
		if(1 == Short_Circuit_Detection()){
				is_material = 0; //无
		}
		uart_send_hmi_is_material(is_material);//1为KT板，2为纤维板
		rt_thread_mdelay(5);

}




/* 纸币 检测 */
void Money_Detection(void)
{

	
		Paper.Capacitance = get_single_capacity();/* 获取单次 电容值*/
		if(0x01 == Money_Debug_Write_Button){
				Money.RMB_100 = Paper.Capacitance;
		}
		else if(0x02 == Money_Debug_Write_Button){
				Money.RMB_50 = Paper.Capacitance;
		}
		else if(0x03 == Money_Debug_Write_Button){
				Money.RMB_10 = Paper.Capacitance;
		}
		else if(0x04 == Money_Debug_Write_Button){
				Money.RMB_5 = Paper.Capacitance;
		}
		if(0x00 != Money_Debug_Write_Button){
				Flash_Update();				
				Buzzer_Set(&Beep,1,1);		
		}
		Money_Debug_Write_Button = 0;
		if(Money_Button == 1){//当检测按钮按下 
				Buzzer_Set(&Beep,1,1);//蜂鸣器响一声
				if( is_in_range(Paper.Capacitance,Money.RMB_100,40) )	{//比较更靠近谁
						RMB_Value = 1; //100元
				}
				else if( is_in_range(Paper.Capacitance,Money.RMB_50,40) ) {
						RMB_Value = 2; //50元
				}
				else if( is_in_range(Paper.Capacitance,Money.RMB_10,40) ) {
						RMB_Value = 3; //50元
				}
				else if( is_in_range(Paper.Capacitance,Money.RMB_5,40) ) {
						RMB_Value = 4; //50元
				}
				else{
					  RMB_Value = 0; //无
				}
				Money_Button = 0;
		}
		
		if(Short_Circuit_Detection() == 1){
				RMB_Value = 0; //无
		}
		uart_send_hmi_is_money(RMB_Value);//1为KT板，2为纤维板	
		rt_thread_mdelay(5);


}

/**
  * @brief  get_single_capacity 获取极板容值
  * @param  None
  * @retval 电容值
  * @notice 
  */
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
				(FDC2214_Paper_Data[0] <= (FDC2214_Data_In_Flash[0]+50) && HMI_Status_Flag == 1)  \
				 || (Paper.Capacitance <= (FDC2214_Data_In_Flash[0]+50) && HMI_Status_Flag == 2) \
				 || (Paper.Capacitance <= (FDC2214_Data_In_Flash[0]+50) && HMI_Status_Flag == 3) 
		
		)
		{
				Paper.ShortStatus = 0x01;//判定短路
				Paper.PaperNumber = 0; //如果短路即为0
				Bling_Set(&Light_Red,100,50,0.5,0,77,0);//红灯提示短路
		}			
		else{
				Bling_Set(&Light_Green,100,50,0.5,0,78,0);//绿灯提示不短路
				Paper.ShortStatus = 0x02; //判定不短路
		}
		uart_send_hmi_is_short(); //发送是否短路
		return Paper.ShortStatus;
}		

float CapacitanceDP= 0;

/**
  * @brief  DataSubsection 电容值分割，获取区间
  * @param  Cap_Division电容切分后的值、arrey事先存入的电容值
  * @retval 
  * @notice 
  */
void DataSubsection(float Cap_Division[],float arrey[])
{

		static int rec = 1;
		
		for(int i = 2;i <= 30;i++){
				CapacitanceDP = (arrey[i-1]-arrey[i]) /2.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;

		}
		for(int i=31;i<=40;i++){
				CapacitanceDP = Div_Parameter.Div_30_40*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=41;i<=50;i++){
				CapacitanceDP = Div_Parameter.Div_40_50*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		
		for(int i=51;i<=60;i++){
				CapacitanceDP = Div_Parameter.Div_50_60*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=61;i<=70;i++){
				CapacitanceDP = Div_Parameter.Div_60_70*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=71;i<=80;i++){
				CapacitanceDP = Div_Parameter.Div_70_80*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}
		for(int i=81;i<=90;i++){
				CapacitanceDP = Div_Parameter.Div_80_90*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}				
		for(int i=91;i<=100;i++){
				CapacitanceDP = Div_Parameter.Div_90_100*(arrey[i-1]-arrey[i]) /100.0f;
				Cap_Division[i-1]= arrey[i-1]-CapacitanceDP;
		}				
		if(rec==1){
				Cap_Division[0] =arrey[1]+(arrey[1]-arrey[2]) /2.0f;
				rec = 0;
		}
}

int Probability_Max = 0;
/**
  * @brief  ProbablityCapacitance 根据分割函数，获取可能的纸张数量
  * @param  50组电容值地址
  * @retval 电容值
  * @notice 按最大百分比返回对应的纸张数，因此只是可能值
  */
uint8 ProbablityCapacitance(float CompareArrey[])	//传入 需要比较的数据
{

		memset(Cap_Probability,0,sizeof(Cap_Probability));//清空电容值落点可能性
	
		for(int i=0;i <= Level;i++ ){
				for(int j=0; j<10 ;j++){
						if( (CompareArrey[j] < Cap_Division[i])  && (CompareArrey[j] >= Cap_Division[i+1])){
								Cap_Probability[i]++;
						}
				}
		}
		for(int n = 0;n < Level ;n++){
				if(Cap_Probability[n] > Cap_Probability[Probability_Max]){
						Probability_Max = (n + 1);
				}
				if(Cap_Probability[0] >= 5){	//纸张数的特殊处理
						Probability_Max = 1;
				}	
		}
		
		if(0 == Cap_Probability[Probability_Max-1]){
				Probability_Max = 0;
		}
		return Probability_Max;
}

