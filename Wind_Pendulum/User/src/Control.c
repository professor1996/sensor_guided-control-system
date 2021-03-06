#include "Control.h"

u8 Mode = 0;

float PWM1 , PWM2;
float R = 30.0 ;
float POWER_MAX = 2700 ;


void Mode_0(void)         //静止
{
	MotorPower(1 , 1);
}

void Mode_1(void)          //沿Y轴自由摆动，长度不短于50cm
{
	const float priod = 2000.0;  //单摆周期(毫秒)
	static uint32_t MoveTimeCnt = 0;
  int A1 = 200;
	float Normalization = 0.0;
	float Omega = 0.0;
	float set_x = 0.0;
	
	MoveTimeCnt += 5;							 //�22ms运算1次
	Normalization = (float)MoveTimeCnt / priod;	 //对单摆周期归一化
	Omega =   Normalization ;            //2.0*3.14159*Normalization;			 //对2π进行归一化处理
	//A = atan((R/88.0f))*57.2958f;				 //根据摆幅求出角度A,88为摆杆距离地面长度cm
	set_x = A1*sin(Omega);                        //计算出当前摆角 	
		
	SetPoint_x = set_x;			//X方向PID定位目标值0
	Proportion_x = 5;	
	Integral_x = 2;	 
	Derivative_x = 5;
	
	SetPoint_y = 0 ;		//Y方向PID跟踪目标值sin
	Proportion_y = 5;	   //60
	Integral_y = 2;	 
	Derivative_y = 5; 
	
	PWM1 = PID_M1_PosLocCalc(Angle_X);	//Pitch
	PWM2 = PID_M2_PosLocCalc(Angle_Y); //Roll

	
	if(PWM1 > POWER_MAX)  PWM1 =  POWER_MAX;
	if(PWM1 < -POWER_MAX) PWM1 = -POWER_MAX;	
	
	if(PWM2 > POWER_MAX)  PWM2 = POWER_MAX;
	if(PWM2 < -POWER_MAX) PWM2 = -POWER_MAX;		
	
	MotorPower(PWM1 , PWM2);
}

 int A2 = 0;

void Mode_2(void)          //沿Y轴自由摆动，长度可调 30cm-50cm
{
	const float priod = 2000.0;  //单摆周期(毫秒)
	static uint32_t MoveTimeCnt = 0;

	float Normalization = 0.0;
	float Omega = 0.0;
	float set_y = 0.0;
	
	MoveTimeCnt += 5;							 //�22ms运算1次
	Normalization = (float)MoveTimeCnt / priod;	 //对单摆周期归一化
	Omega =   Normalization ;            //2.0*3.14159*Normalization;			 //对2π进行归一化处理
	//A = atan((R/88.0f))*57.2958f;				 //根据摆幅求出角度A,88为摆杆距离地面长度cm
	set_y = A2*sin(Omega);                        //计算出当前摆角 	
	
	SetPoint_x = 0;			//X方向PID定位目标值0
	Proportion_x = 4;	
	Integral_x = 1.7;	 
	Derivative_x = 5;
	
	SetPoint_y = set_y ;		//Y方向PID跟踪目标值sin
	Proportion_y = 4;	   //60
	Integral_y = 1.7;	 
	Derivative_y = 5; 
	
	PWM1 = PID_M1_PosLocCalc(Angle_X);	//Pitch
	PWM2 = PID_M2_PosLocCalc(Angle_Y); //Roll

	
	if(PWM1 > POWER_MAX)  PWM1 =  POWER_MAX;
	if(PWM1 < -POWER_MAX) PWM1 = -POWER_MAX;	
	
	if(PWM2 > POWER_MAX)  PWM2 = POWER_MAX;
	if(PWM2 < -POWER_MAX) PWM2 = -POWER_MAX;		
	
	MotorPower(PWM1 , PWM2);
}

int A3 = 100;
int B3 = 100;

void Mode_3(void)               //可设定摆动角度0-360° ， 不短于20cm
{
	const float priod = 2000.0;  //单摆周期(毫秒)
	static uint32_t MoveTimeCnt = 0;

	float Normalization = 0.0;
	float Omega = 0.0;
	float set_x = 0.0;
	float set_y = 0.0;
	
	MoveTimeCnt += 2;							 //�22ms运算1次
	Normalization = (float)MoveTimeCnt / priod;	 //对单摆周期归一化
	Omega =   Normalization ;            //2.0*3.14159*Normalization;			 //对2π进行归一化处理
	//A = atan((R/88.0f))*57.2958f;				 //根据摆幅求出角度A,88为摆杆距离地面长度cm
	set_x = A3*sin(Omega);                        //计算出当前摆角 	
	set_y = B3*sin(Omega);                        //计算出当前摆角
	
	SetPoint_x = set_x;			//X方向PID定位目标值0
	Proportion_x = 4;	
	Integral_x = 1.7;	 
	Derivative_x = 5;
	
	SetPoint_y = set_y ;		//Y方向PID跟踪目标值sin
	Proportion_y = 4;	   //60
	Integral_y = 1.7;	 
	Derivative_y = 5; 
	
	PWM1 = PID_M1_PosLocCalc(Angle_X);	//Pitch
	PWM2 = PID_M2_PosLocCalc(Angle_Y); //Roll

	
	if(PWM1 > POWER_MAX)  PWM1 =  POWER_MAX;
	if(PWM1 < -POWER_MAX) PWM1 = -POWER_MAX;	
	
	if(PWM2 > POWER_MAX)  PWM2 = POWER_MAX;
	if(PWM2 < -POWER_MAX) PWM2 = -POWER_MAX;		
	
	MotorPower(PWM1 , PWM2);
}

void Mode_4(void)         // 拉起一定角度放下，5s内恢复原位置（0，0）
{
	if(Angle_X<45.0 && Angle_Y<45.0)	//小于45度才进行制动
	{		
		SetPoint_x = 0;			//X方向PID定位目标值0
		Proportion_x = 20;	
		Integral_x = 0;	 
		Derivative_x = 70;

		SetPoint_y = 0 ;		//Y方向PID跟踪目标值sin
		Proportion_y = 20;	   //60
		Integral_y = 0;	 
		Derivative_y = 70; 
			
	  PWM1 = PID_M1_PosLocCalc(Angle_X);	//Pitch
	  PWM2 = PID_M2_PosLocCalc(Angle_Y); //Roll
		
	if(PWM1 > POWER_MAX)  PWM1 =  POWER_MAX;
	if(PWM1 < -POWER_MAX) PWM1 = -POWER_MAX;	
	
	if(PWM2 > POWER_MAX)  PWM2 = POWER_MAX;
	if(PWM2 < -POWER_MAX) PWM2 = -POWER_MAX;		
	
	}
	else	
	{
	 	PWM1 = 1;
		PWM2 = 1;	
	}
	
	MotorPower(PWM1 , PWM2);
}

int A5 = 110;

void Mode_5(void)         //画圆， 半径在15cm-45cm内可调
{
		const float priod = 2000.0;  //单摆周期(毫秒)
	static uint32_t MoveTimeCnt = 0;

	float Normalization = 0.0;
	float Omega = 0.0;
	float set_x = 0.0;
	float set_y = 0.0;
	
	MoveTimeCnt += 2;							 //�22ms运算1次
	Normalization = (float)MoveTimeCnt / priod;	 //对单摆周期归一化
	Omega =   Normalization ;            //2.0*3.14159*Normalization;			 //对2π进行归一化处理
	//A = atan((R/88.0f))*57.2958f;				 //根据摆幅求出角度A,88为摆杆距离地面长度cm
	set_x = A5*sin(Omega);                        //计算出当前摆角 	
	set_y = A5*sin(Omega + 3.14159 / 2);                        //计算出当前摆角
	
	SetPoint_x = set_x;			//X方向PID定位目标值0
	Proportion_x = 4;	
	Integral_x = 1.7;	 
	Derivative_x = 5;
	
	SetPoint_y = set_y ;		//Y方向PID跟踪目标值sin
	Proportion_y = 4;	   //60
	Integral_y = 1.7;	 
	Derivative_y = 5; 
	
	PWM1 = PID_M1_PosLocCalc(Angle_X);	//Pitch
	PWM2 = PID_M2_PosLocCalc(Angle_Y); //Roll

	
	if(PWM1 > POWER_MAX)  PWM1 =  POWER_MAX;
	if(PWM1 < -POWER_MAX) PWM1 = -POWER_MAX;	
	
	if(PWM2 > POWER_MAX)  PWM2 = POWER_MAX;
	if(PWM2 < -POWER_MAX) PWM2 = -POWER_MAX;		
	
	MotorPower(PWM1 , PWM2);
}

void Mode_6(void)
{
	
}






void MotorPower(float pwm1 , float pwm2)
{
	
	if(pwm1 < 0)
	{
	 	PWM_M2_Forward(-pwm1);
		PWM_M3_Forward(0);
	}
	else if(pwm1 > 0)
	{
	 	PWM_M2_Forward(0);
		PWM_M3_Forward(pwm1);	
	}

	if(pwm2 < 0)
	{
	 	PWM_M4_Forward(-pwm2);
		PWM_M1_Forward(0);
	}
	else if(pwm2 > 0)
	{
	 	PWM_M4_Forward(0);
		PWM_M1_Forward(pwm2);	
	} 	
}









