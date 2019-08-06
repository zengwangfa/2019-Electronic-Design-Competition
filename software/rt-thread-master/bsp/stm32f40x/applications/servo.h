#ifndef __SERVO_H
#define __SERVO_H

#include "drv_pwm.h"

typedef struct  /*  */
{
		short MaxValue;		//舵机 正向最大值
		short MinValue;	  //舵机 反向
		short MedValue;   //舵机 中值
		short CurrentValue; //舵机当前值
		uint8 Speed;		  //舵机速度值
}ServoType;


void Servo_Dir_Control(short offest); //PWM越大 ,往右
void RoboticArm_Control(uint8 *action);

void YunTai_Control(uint8 *action);
void DirectionProportion(int Mode);
/* 云台控制 

当前x，期望x;当前y，期望y
*/
void yuntai_pid_control(float present_x,float expect_x,float persent_y,float expect_y) ;//

extern ServoType RoboticArm ;//机械臂

extern ServoType YunTai; //云台

extern int DirectionMode;


#endif
