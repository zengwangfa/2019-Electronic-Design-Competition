#ifndef __OLED_H_
#define __OLED_H_

#include "DataType.h"
#include "DataProcess.h"
void Boot_Animation(void);//开机动画
void menu_define(void); //菜单定义
void OLED_StatusPage(void);
void OLED_WorkPage(void);
void OLED_DebugPage(void);
void OLED_LockPage(void);
void OLED_SwitchPage(void);
/* 打印机 页面*/
void OLED_PrintPage(void);
void OLED_MaterPage(void);
void OLED_FuncSwitchPage(void);
//void OLED_GyroscopePage(void);
void draw_circle(uint8 x,uint8 y,uint8 r); //圆心(x,y),半径r
void draw_line(uint8 x0,uint8 y0,float k,uint8 dot); //过固定点(x0,y0),斜率k  dot:0,清空;   1,填充	  
void draw_fill_circle(uint8 x0,uint8 y0,uint8 r,uint8 dot);//写画实心圆心(x0,y0),半径r

void OLED_Clear(void);

typedef struct {
		uint8 pagenum;  		//页码
		uint8 pagechange;
	  uint8 pagechange_flag;
		char *pagename[10]; //页名
}Oled_Type;


typedef enum
{
		SwitchPage = 0,
		DebugPage = 1, //页码从第一页开始
		WorkPage,
		FuncPage,
		PrintPage,
		MaterPage,   

	
		OLED_Page_MAX   //页码最大值
	
}MENU_LIST_Enum; //菜单枚举

extern Oled_Type oled;
extern volatile MENU_LIST_Enum MENU;

#endif


