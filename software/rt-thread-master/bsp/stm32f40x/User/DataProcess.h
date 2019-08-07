#ifndef __DATA_PROCESS_H_
#define __DATA_PROCESS_H_

#include "DataType.h"

void get_capcity_value(void);

/* 获取上极板 容值*/
float get_top_capacity(void);

/* 获取下极板 容值 */
float get_bottom_capacity(void);



void FDC2214_Data_Adjust(void);//数据校准

void Uart_Send_HMI(uint8 *cmd);
#endif



