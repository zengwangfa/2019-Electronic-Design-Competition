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
	
	int Time; //时间

	
}PaperCountEngine_Type;//纸张测数器状态



void get_capcity_value(void);

/* 【校准时】获取单极板 容值 */
float get_single_capacity(void);

void FDC2214_Data_Adjust(void);//数据校准

void	Capcity_Paper_Detection(void); //获取电容值

int	Short_Circuit_Detection(void);//短路检测

void DataSubsection(float Cap_Division[],float arrey[],int Number);

uint8 ProbablityCapacitance(float CompareArrey[])	;


extern float Cap_Division[101];/**/

extern int HMI_Debug_Write_Button;
extern int HMI_Work_Button;

extern PaperCountEngine_Type Paper; //SB
extern int ShortFlag;        //短路标志位


extern float KT_Board_Value_In_Flash; //KT板Flash数值
extern float Fiber_Board_Value_In_Flash;//纤维板 Flash数值

extern float Money_100_In_Flash ; //100元
extern float Money_50_In_Flash ;  //50元

extern int Div_40_50_Parameter; //切分 40~50张的系数
extern int Div_50_60_Parameter; //
extern int Div_60_70_Parameter; //
extern int RMB_Value;

/* 打印机 纸张检测 */
void Printer_Paper_Detection(void);

/* 材料 检测 */
void Material_Detection(void);

/* 材料 检测 */
void Money_Detection(void);

#endif



