
/*
 * key.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  IO 设备控制（KEY、BUZZER）
 */


#define LOG_TAG  "ioDevices"//Tag

#include <drivers/pin.h>
#include <elog.h>
#include "ioDevices.h"
#include "oled.h"
#include "HMI.h"
#include "EasyThread.h"
/*---------------------- Constant / Macro Definitions -----------------------*/

#define KEY_MIN	  				  58	 //PF5
#define KEY_ADD 					  60	 //PF4  
#define NEXT_PIN  				  64 	 //下一页
#define Work_Button_PIN 		66	 //确认

#define key_min_read 					rt_pin_read(KEY_MIN)
#define key_add_read 					rt_pin_read(KEY_ADD)
#define next_oled_read 			  rt_pin_read(NEXT_PIN)
#define work_button_read 			rt_pin_read(Work_Button_PIN)

#define BOMA3_PIN 					13	 //PF3   拨码开关IO
#define BOMA2_PIN	  				15	 //PF5
#define BOMA1_PIN 					14	 //PF4  

#define Buzzer_PIN 					59   //

#define boma1_read 					rt_pin_read(BOMA1_PIN)
#define boma2_read 					rt_pin_read(BOMA2_PIN)
#define boma3_read 					rt_pin_read(BOMA3_PIN)

#define wifi_read 					rt_pin_read(WIFI_CONNECT_PIN)
/*----------------------- Variable Declarations -----------------------------*/
/* ALL_init 事件控制块. */

Buzzer_Type Beep;  //蜂鸣器控制器


/*----------------------- Function Implement --------------------------------*/


int ioDevices_thread_init(void)
{
   rt_thread_t ioDecices_tid;

    ioDecices_tid = rt_thread_create("ioDev",ioDevices_thread_entry, RT_NULL,1024,8, 10);

    if (ioDecices_tid != RT_NULL){			
				rt_pin_mode(NEXT_PIN , PIN_MODE_INPUT_PULLUP);    //功能按键、拨码开关  上拉输入
				rt_pin_mode(KEY_MIN, PIN_MODE_INPUT_PULLUP);  //拨码开关  上拉输入
				rt_pin_mode(KEY_ADD, PIN_MODE_INPUT_PULLUP);  
				rt_pin_mode(Work_Button_PIN, PIN_MODE_INPUT_PULLUP);  

				rt_pin_mode (Buzzer_PIN, PIN_MODE_OUTPUT);  //输出模式
				rt_pin_write(Buzzer_PIN, PIN_HIGH);

//				rt_pin_attach_irq(NEXT_PIN, PIN_IRQ_MODE_RISING_FALLING, next_oled, RT_NULL);/* 绑定中断，上升沿模式，回调函数名为key_down */
//				rt_pin_irq_enable(NEXT_PIN, PIN_IRQ_ENABLE);/* 使能中断 */
//			
//				rt_pin_attach_irq(KEY_MIN, PIN_IRQ_MODE_RISING_FALLING, paper_min, RT_NULL);/* 绑定中断，上升沿模式，回调函数名为key_down */
//				rt_pin_irq_enable(KEY_MIN, PIN_IRQ_ENABLE);/* 使能中断 */

//				rt_pin_attach_irq(KEY_ADD, PIN_IRQ_MODE_RISING_FALLING, paper_add, RT_NULL);/* 绑定中断，上升沿模式，回调函数名为key_down */
//				rt_pin_irq_enable(KEY_ADD, PIN_IRQ_ENABLE);/* 使能中断 */
//			
//				rt_pin_attach_irq(Work_Button_PIN, PIN_IRQ_MODE_RISING_FALLING, work_button, RT_NULL);/* 绑定中断，上升沿模式，回调函数名为key_down */
//				rt_pin_irq_enable(Work_Button_PIN, PIN_IRQ_ENABLE);/* 使能中断 */


				log_i("IoDev_Init()");
				rt_thread_startup(ioDecices_tid);
		}
		return 0;
}
INIT_APP_EXPORT(ioDevices_thread_init);


/* get 2位拨码值 */
uint8 get_boma_value(void)
{
    static uint8 val = 0; //reserve(存储)
    
		val = boma1_read *1 + boma2_read *2 + boma3_read *4 + 1; //得到8种状态(1~8)
    return val;
}



void Key_Read(void)
{
		if(0 == work_button_read){
				rt_thread_mdelay(100);
				if(oled.pagenum == 1){
						HMI_Debug_Write_Button = 1;
				}
				else if(oled.pagenum == 2){
						HMI_Work_Button = 1;
				}
		}
		

		if(0 == next_oled_read){
				rt_thread_mdelay(300);
				oled.pagenum += 1;				
		}


		if(0 == key_add_read ){
				rt_thread_mdelay(200);
				if(oled.pagenum == 1){
						 HMI_Page_Number ++;	
						if(HMI_Page_Number >= 50){
								HMI_Page_Number = 50;
						}	
				}
		}

		if(0 == key_min_read){
				rt_thread_mdelay(200);
				if(oled.pagenum == 1){
						HMI_Page_Number --;
						if(HMI_Page_Number <= 0){
								HMI_Page_Number = 0;
						}
				}
		}

}








//void work_button(void *args)
//{
//			rt_thread_mdelay(1000);
//		if(oled.pagenum == 1){
//				HMI_Debug_Write_Button = 1;
//		
//		}
//		else if(oled.pagenum == 2){
//				HMI_Work_Button = 1;
//		}
//		
//}



///* 按键按下产生的任务 */
//void next_oled(void *args)  
//{
//		rt_thread_mdelay(1000);
//		oled.pagenum += 1;				

//}


///* 按键按下产生的任务 */
//void paper_add(void *args)  
//{
//			rt_thread_mdelay(1000);
//		if(oled.pagenum == 1){
//				 HMI_Page_Number ++;	
//				if(HMI_Page_Number >= 50){
//						HMI_Page_Number = 50;
//				}	
//		}

//}


///* 按键按下产生的任务 */
//void paper_min(void *args)  
//{
//			rt_thread_mdelay(1000);
//		if(oled.pagenum == 1){
//				HMI_Page_Number --;
//				if(HMI_Page_Number <= 0){
//						HMI_Page_Number = 0;
//				}
//		}
//}


/*buzzer为蜂鸣器控制器  count为响的次数  length响的时间长度  */
void Buzzer_Set(Buzzer_Type* buzzer,uint8_t count,uint8_t length)
{
		buzzer->count  = count *2;	//响与不向为 两次状态，所以 *2
		buzzer->time   = length*5; //保存时间间隔长度
		buzzer->number = length*5; //计时间长度
}  

/* 蜂鸣器鸣响任务【可指示系统各个状态】 */
void Buzzer_Process(Buzzer_Type * buzzer)
{
		if(buzzer->count >= 1)//如果响的次数大于等于1
		{  
				if(buzzer->number >= 1){
						if(1 == buzzer->count%2)   {rt_pin_write(Buzzer_PIN, PIN_LOW);}//【响一声】
						else                    	{rt_pin_write(Buzzer_PIN, PIN_HIGH) ;} 
						buzzer->number--;	//时间间隔 减1
				}
				else{
						buzzer->number = buzzer->time;//响一次完成 后重新赋值
						buzzer->count--;  //次数减1
				}
		}
		else
		{
				rt_pin_write(Buzzer_PIN, PIN_HIGH); //完成后拉低【不响】
				buzzer->time  = 0;
				buzzer->count = 0;
		}
}







