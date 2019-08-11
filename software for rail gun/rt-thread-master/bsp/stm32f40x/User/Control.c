/*
 * Control.c
 *
 *  Created on: 2019年3月20日
 *      Author: zengwangfa
 *      Notes:  运动总控制
 */
 
#define LOG_TAG "Control"

#include <rtthread.h>
#include <elog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "Control.h"
#include "PID.h"
#include "rc_data.h"

#include "focus.h"
#include "led.h"
#include "servo.h"
#include "PropellerControl.h"
#include "propeller.h"
#include "sensor.h"

Rectange_Type Rectange = {100,100};
Star_Type Star = {0,0};
Cycle_Type Cycle = {0,0,50,50};
Trigonometric_Type Sin = {0,0};

ExpectAngle_Type ExpectAngle;

int target_x = 160;//目标期望
int target_y = 120;

int persent_x = 160;//当前坐标
int persent_y = 120;

float target_pit = 0.0f; //期望角度
float target_yaw = 0.0f;

float persent_pit = 0.0f;//当前角度
float persent_yaw = 0.0f;


uint16 Pitch_Axis_Output_Limit_Left(int16 value)
{
		//不超过+500   不超过-500
		value = (value) > PITCH_YUNTAI_MED + 500 ? PITCH_YUNTAI_MED + 500 : value ;//正向限幅
		value = (value) < PITCH_YUNTAI_MED - 500 ? PITCH_YUNTAI_MED - 500 : value;//反向限幅
	
		return value ;
}

uint16 Yaw_Axis_Output_Limit_Right(int16 value)
{
		//不超过+500   不超过-500
		value = (value) > YAW_YUNTAI_MED + 600 ? YAW_YUNTAI_MED + 600: value ;//正向限幅
		value = (value) < YAW_YUNTAI_MED - 600 ? YAW_YUNTAI_MED - 600 : value;//反向限幅
	
		return value ;
}



void ReadExpect(ExpectAngle_Type *ExpectAngle,float x,float y)//期望角度转换
{
		ExpectAngle->yaw_rad = atan(x / STEETMAX); //弧度制
		ExpectAngle->pit_rad = atan(y / STEETMAX);
		ExpectAngle->yaw_deg = Rad2Deg(ExpectAngle->yaw_rad);//转换为角度
		ExpectAngle->pit_deg = Rad2Deg(ExpectAngle->pit_rad);
}

void DrawStar(Star_Type *Star)
{
	static int t = 0;
	static int Mode = 1;

	switch(Mode)
	{
		case 1:t++;Star->x = t*cos(0.4f*PI);
				Star->y = 50 - t*sin(0.4f*PI);
				if(t>=100)
				{
					Mode  = 2;
					t = 0;
				}
				break;
		case 2:t++;Star->x = 31- t*cos(0.2f*PI);
				Star->y = -45 + t*sin(0.2f*PI);
				if(t>=100)
				{
					Mode  = 3;
					t = 0;
				}break;
		case 3:t++;Star->x = -50 +t;
				Star->y = 14 ;
				if(t>=100)
				{
					Mode  = 4;
					t = 0;
				}break;
		case 4:t++;Star->x = 50 - t*cos(0.2f*PI);
				Star->y = 14- t*sin(0.2f*PI);
				if(t>=100)
				{
					Mode  = 5;
					t = 0;
				}break;
		case 5:t++;Star->x = -31 + t*cos(0.4f*PI);
				   Star->y = -45 +  t*sin(0.4f*PI);
				if(t>=100)
				{
					Mode  = 1;
					t = 0;
				}break;
	}
	
}

void DrawRetange(Rectange_Type *Rec)
{
		static int Flag = 1, Size_X,Size_Y;
		if(Flag)
		{
			Flag = 0;
			Size_X = Rec->x;
			Size_Y = Rec->y;
		}
		if(Rec->y>=Size_Y)
		{
			Rec->x--;
		}
		if(Rec->x <= -Size_X)
		{
			Rec->y--;		
		}
		if(Rec->y<=-Size_Y)
		{
			Rec->x++;

		}
		if(Rec->x>=Size_X)
		{
			Rec->y++;
		}

}
void DrawCyc(Cycle_Type *cyc)
{

		
		cyc->Angle += 0.01f;//( Deg2Rad(fabs(sin(2*cyc->Angle+(PI/2)))) + 0.01f);
	
//		if(cyc->Angle>= 2*PI){
//				cyc->Angle = 0;
//		}
		cyc->x = cyc->a * cos(cyc->Angle);
		cyc->y = cyc->b * sin(cyc->Angle);
	
		//sprintf(str,"x%fy%f\n",cyc->x,cyc->y);
	
   // rt_kprintf(str);
}

void DrawSin(Trigonometric_Type *Sin)
{
		Sin->x += 1;
		Sin->y = 100*sin(Sin->x/100);
}

/* 寻找色块 */
void Yuntai_Find_Blobs(void)
{

		target_x = get_target_x();//获取 指示点X轴
	  target_y = get_target_y();//获取 指示点Y轴
	
		persent_x = get_persent_x();//获取 
	  persent_y = get_persent_y();//获取 
	
		yuntai_pid_control(persent_x,target_x,persent_y,target_y);

 		PropellerPower.leftMiddle  = YAW_YUNTAI_MED   - Total_Controller.Yaw_Angle_Control.Control_OutPut;   //水平
	  PropellerPower.rightMiddle = PITCH_YUNTAI_MED + Total_Controller.Pitch_Angle_Control.Control_OutPut; //俯仰    左边是  上面控制俯仰的舵机 Y轴
	
}



extern int Rail_Gun_Distance;
extern int Rail_Gun_Angle;
float init_yaw = 0.0f;
 
int last_lefeMiddle = 0;
int last_rightMiddle = 0;
int yuntai_mode = 0;
int rail_gun_high = 50;

void Close_Loop_Angle_Control(void)
{

		static uint8 ON_OFF = 0;//自锁开关
		if(0 == ON_OFF ){
				ON_OFF = 1;
				init_yaw = Sensor.JY901.Euler.Yaw; //获取初始
				log_i("init_yaw:%f",init_yaw);
		}
		
		last_lefeMiddle = PropellerPower.leftMiddle;
	  last_rightMiddle = PropellerPower.rightMiddle;
		
		switch(yuntai_mode){ //默认为期望为0，则自稳状态
		
				case 1:	DrawCyc(&Cycle);
								ReadExpect(&ExpectAngle,Cycle.x,Cycle.y);
								break;
				case 2: DrawStar(&Star);
								ReadExpect(&ExpectAngle,Star.x,Star.y);
								break;
				case 3: DrawRetange(&Rectange);
							  ReadExpect(&ExpectAngle,Rectange.x,Rectange.y);
								break;
				case 4: DrawSin(&Sin);
				      	ReadExpect(&ExpectAngle,Sin.x,Sin.y);
				default:break;
		}
	
		target_yaw = Rail_Gun_Angle;
		//target_yaw = ExpectAngle.yaw_deg + init_yaw;
		persent_yaw = Sensor.JY901.Euler.Yaw;
		
		if(Rail_Gun_Distance >= 250){
				ExpectAngle.pit_deg = -30;
		
		}else{
				ExpectAngle.pit_deg = -15;

		}

		
		target_pit = ExpectAngle.pit_deg;
		persent_pit = Sensor.JY901.Euler.Pitch;

		yuntai_pid_control(persent_yaw,target_yaw,persent_pit,target_pit);		
	
		PropellerPower.leftMiddle  = YAW_YUNTAI_MED   + Total_Controller.Yaw_Angle_Control.Control_OutPut;   //水平 Yaw
	  PropellerPower.rightMiddle = PITCH_YUNTAI_MED + Total_Controller.Pitch_Angle_Control.Control_OutPut; //俯仰    左边是  上面控制俯仰的舵机 Y轴	


}

float init_pit = 0.0f;
void Car_Pitch_Control(void)
{

	
		static uint8 ON_OFF = 0;//自锁开关
		if(0 == ON_OFF ){
				ON_OFF = 1;
				init_pit = Sensor.JY901.Euler.Pitch; //获取初始
		}
		
		target_yaw =  init_yaw;
		persent_yaw = Sensor.JY901.Euler.Yaw;
		
		target_pit = ExpectAngle.pit_deg + init_pit;
		persent_pit = Sensor.JY901.Euler.Pitch;

		yuntai_pid_control(persent_yaw,target_yaw,persent_pit,target_pit);		
	
		Back_Wheel_Control( Total_Controller.Pitch_Angle_Control.Control_OutPut);//后轮控制
		//PropellerPower.rightUp  = YAW_YUNTAI_MED   + Total_Controller.Yaw_Angle_Control.Control_OutPut;   //水平 Yaw
	  //PropellerPower.rightDown = Total_Controller.Pitch_Angle_Control.Control_OutPut; //俯仰    左边是  上面控制俯仰的舵机 Y轴	


}




uint8 servo_test_flag = 0;//是否开启 舵机中值调试 flag
void Two_Axis_Yuntai_Control(void)
{
	  
	
	
		if(0 == servo_test_flag){
				Close_Loop_Angle_Control();
				//Yuntai_Find_Blobs();
				PropellerPower.leftMiddle  = Yaw_Axis_Output_Limit_Right(PropellerPower.leftMiddle);  //云台
				PropellerPower.rightMiddle = Pitch_Axis_Output_Limit_Left(PropellerPower.rightMiddle);		
		}
	



	
	
		TIM4_PWM_CH1_D12(PropellerPower.leftMiddle);  //左中   D12
		TIM4_PWM_CH2_D13(PropellerPower.rightMiddle); //右中   D13
}



static int set_rail_gun_high(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage:");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 1000){
				rail_gun_high = atoi(argv[1]);
		}

		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(set_rail_gun_high,ag: yaw );


static int set_mode(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage:");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 10){
				yuntai_mode = atoi(argv[1]);
		}

		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(set_mode,ag: yaw );

static int set_cycle_r(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage:");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 5000){
				Cycle.a = atoi(argv[1]);
			  Cycle.b = atoi(argv[1]);
		}

		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(set_cycle_r,ag: yaw );

static int expect_yaw(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage:");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 200000){
				target_yaw = target_yaw + atoi(argv[1]);
		}

		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(expect_yaw,ag: yaw );


static int expect_pitch(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage:");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1]) <= 200000){
				target_pit = atoi(argv[1]);
		}

		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(expect_pitch,ag: yaw );

static int servo_med_value_test_on_off(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage: servo_med_value_on_off <on/off>");
				result = -RT_ERROR;
        goto _exit;
    }
		if(!strcmp(argv[1],"off")){
				servo_test_flag = 1;
		}
		else if(!strcmp(argv[1],"on")){
				servo_test_flag = 0;
		}
		else {
				log_e("Error! The value is out of range!");
		}
_exit:
    return result;
}
MSH_CMD_EXPORT(servo_med_value_test_on_off,ag: servo_med_value_on_off <on/off>);


/*【深度 】期望yaw MSH方法 */
static int servo_med_value_test(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        log_e("Error! Proper Usage: coords pit yaw");
				result = -RT_ERROR;
        goto _exit;
    }

		if(atoi(argv[1])<5000 && atoi(argv[2])<5000  ){
				PropellerPower.leftMiddle  = atoi(argv[1]);
				PropellerPower.rightMiddle = atoi(argv[2]);	
		}
		else {
				log_e("Error! The  value is out of range!");
		}

		
_exit:
    return result;
}
MSH_CMD_EXPORT(servo_med_value_test,ag: coords 50 50);


























void Depth_PID_Control(float expect_depth,float sensor_depth)
{
		
		Total_Controller.High_Position_Control.Expect = expect_depth ; //期望深度由遥控器给定
		Total_Controller.High_Position_Control.FeedBack = sensor_depth;  //当前深度反馈
		PID_Control(&Total_Controller.High_Position_Control);//高度位置控制器
	
		robot_upDown(&Total_Controller.High_Position_Control.Control_OutPut);		//竖直推进器控制
}



void Gyro_Control(void)//角速度环
{

//  	偏航角前馈控制
//  	Total_Controller.Yaw_Gyro_Control.FeedBack=Yaw_Gyro;


//		PID_Control_Div_LPF(&Total_Controller.Yaw_Gyro_Control);
//		Yaw_Gyro_Control_Expect_Delta=1000*(Total_Controller.Yaw_Gyro_Control.Expect-Last_Yaw_Gyro_Control_Expect)
//			/Total_Controller.Yaw_Gyro_Control.PID_Controller_Dt.Time_Delta;
//		//**************************偏航角前馈控制**********************************
//		Total_Controller.Yaw_Gyro_Control.Control_OutPut+=Yaw_Feedforward_Kp*Total_Controller.Yaw_Gyro_Control.Expect
//			+Yaw_Feedforward_Kd*Yaw_Gyro_Control_Expect_Delta;//偏航角前馈控制
//		Total_Controller.Yaw_Gyro_Control.Control_OutPut=constrain_float(Total_Controller.Yaw_Gyro_Control.Control_OutPut,
//																																		 -Total_Controller.Yaw_Gyro_Control.Control_OutPut_Limit,
//																																		 Total_Controller.Yaw_Gyro_Control.Control_OutPut_Limit);
//		Last_Yaw_Gyro_Control_Expect=Total_Controller.Yaw_Gyro_Control.Expect;
//		

}

/*【深度 】期望yaw MSH方法 */
static int depth(int argc, char **argv)
{
    int result = 0;
    if (argc != 2){
        rt_kprintf("Error! Proper Usage: RoboticArm_openvalue_set 1600");
				result = -RT_ERROR;
        goto _exit;
    }
		if(atoi(argv[1])<100){
				Expect_Depth = atoi(argv[1]);
		}
		else {
				log_e("Error! The  value is out of range!");
		}

		
_exit:
    return result;
}
MSH_CMD_EXPORT(depth,ag: depth 10);





/*【解锁】 修改MSH方法 */
static int unlock(int argc, char **argv) //只能是 0~3.0f
{
		ControlCmd.All_Lock = UNLOCK;
		return 0;
}
MSH_CMD_EXPORT(unlock,unlock);


/*【锁定】 修改MSH方法 */
static int lock(int argc, char **argv) //只能是 0~3.0f
{
		ControlCmd.All_Lock = LOCK;
		return 0;
}
MSH_CMD_EXPORT(lock,lock);

