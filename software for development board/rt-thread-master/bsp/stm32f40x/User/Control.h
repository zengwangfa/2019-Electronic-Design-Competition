#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "DataType.h"
#include "rc_data.h"

#define STEETMAX 250
//#define PITCH_YUNTAI_MED  1900
//#define YAW_YUNTAI_MED    1400

//#define PITCH_YUNTAI_MED  2200 //3D 打印的 银燕舵机云台
//#define YAW_YUNTAI_MED    1500

#define PITCH_YUNTAI_MED  1720 //买了的结构 银燕舵机云台
#define YAW_YUNTAI_MED    1650
typedef struct 
{
	float x;
	float y;
	float a;
	float b;
	float Angle;

}Cycle_Type;// 圆

typedef struct
{
	float x;
	float y;
}Star_Type; //五角星

typedef struct
{
	float x;
	float y;
}Trigonometric_Type;//三角函数

typedef struct
{
	int x;
	int y;	
}Rectange_Type; //矩形

typedef struct
{
	float yaw_rad; //x轴
	float pit_rad;
	float yaw_deg; //x轴
	float pit_deg;

}ExpectAngle_Type;//期望角度


void Car_Pitch_Control(void);
void Angle_Control(void);
void Depth_PID_Control(float expect_depth,float sensor_depth);


void FourtAxis_Control(Rocker_Type *rc);

void SixAxis_Control(Rocker_Type *rc);


void Convert_RockerValue(Rocker_Type *rc); //获取摇杆值

void Speed_Buffer(short *now_value,short *last_value,short BufferRange);


void Two_Axis_Yuntai_Control(void);
#endif


