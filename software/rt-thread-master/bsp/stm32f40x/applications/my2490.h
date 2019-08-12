#ifndef __MY2490_H_
#define __MY2490_H_
#include "DataType.h"

void uart_send_my2490_now_status(uint8 *cmd_array,uint32 number);//串口发送给 MY2490当前纸张数量

extern uint8 my2490_number_array[7] ; //my2490对应曲目
extern uint8 my2490_louder_array[7] ; //my2490  音量+
extern uint8 my2490_lower_array[7]  ; //my2490 音量-

void uart_send_my2490_now_sounds(void);//串口发送给 升级音效
void Volunm_Max(void);  //使音量最大
#endif
