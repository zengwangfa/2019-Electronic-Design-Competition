#ifndef __DATA_PROCESS_H_
#define __DATA_PROCESS_H_

#include "DataType.h"

typedef struct
{
	uint8 Status;      //检测时状态
	uint8 ShortStatus; //短路的状态
	uint8 Finish_Flag;  //读取完成的标志
	uint8 PaperNumber;  //纸张数量
	float Capacitance;

	
}PaperCountEngine_Type;//纸张测数器状态



extern PaperCountEngine_Type Paper; //SB


void get_capcity_value(void);

/* 获取上极板 容值*/
float get_top_capacity(void);

/* 获取下极板 容值 */
float get_bottom_capacity(void);

/* 【校准时】获取单极板 容值 */
float get_single_capacity(void);

void FDC2214_Data_Adjust(void);//数据校准

void Uart_Send_HMI(uint8 *cmd);

void	Get_Capcity_Value(void); //获取电容值

void	Short_Circuit_Detection(void);//短路检测


void DataSubsection(float CapacitanceSubsection[],float arrey[],int Number);

uint8 ProbablityCapacitance(float CompareArrey[])	;
#endif



