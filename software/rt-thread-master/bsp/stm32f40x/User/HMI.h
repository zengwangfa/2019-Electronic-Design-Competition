#ifndef __HMI_H_
#define __HMI_H_

#include "DataType.h"

void HMI_Data_Analysis(uint8 Data); //控制数据解析
uint32 get_set_page_number(void);

uint32 get_set_status(void);

/* 四条发送给HMI的 */
void uart_send_hmi_writer_status(uint8 *cmd);//发送给 hmi写入的状态

void uart_send_hmi_paper_numer(uint8 N_number);  //发送给hmi 纸张数量

void uart_send_hmi_is_short(void); //发送给hmi 是否短路

void uart_send_hmi_reboot(void);//让him复位
/* -------------  */
extern int HMI_Status_Flag ;//串口屏 设定状态标志位 【调试 1】or【工作2】

extern int HMI_Page_Number ;//串口屏发送的校准  纸张数

extern float FDC2214_Data_In_Flash[50];
extern float FDC2214_Paper_Data[50];

extern int HMI_Status_Flag;
#endif


