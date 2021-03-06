/********************************************************
 
  说明：本工程模板包含MPU6050，OLED,TIM2作为定时器使用时间片，
	TIM3作4路PWM输出并计算输出的脉冲个数，使用PC15作为LED，
	使用PE1，PE2，PE3，PE4做外部中断的按键输入。
	
	作者：汕头大学电子系max团队 

********************************************************/
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "math.h"
#include "led.h"   //LED:PC15
#include "TIM4.h" //PB6和PB7为编码器通道
#include "TIM2.h"  //定时器2作为时间片
#include "pwm.h" //定时器3做PWM输出，并计算输出脉冲个数；PB0,PB1,PA6,PA7
//#include "OLED_I2C.h"  //SCL:PB10 SDA:PB11
#include "delay_1.h"
#include "key.h" //按键：PE1,PE2,PE3,PE4
#include "ADC.h"  //AD1：通道1 PC0
#include "DMA.h"

#define N 10

float Kp = 160;
float Kd = 5;
float Ki = 1.2;
float Kp_b = 0;        //编码器Kp
float Ki_b = 0;        //编码器Kd 

int   Go = 0.0;
float err = 0;

float jifen_shangxian = 1000;  //积分限幅
float Ju_Li_Max = 10000;  //积分限幅

// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue;

// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal,Angle,Angle_err,Angle_Last,Angle_jifen;  

extern uint8_t SendBuff[SIZE];

int PWM_OUT = 0,PWM_WEIYI ,PWM_ANGLE;
u8 time = 2; //定时器2计�
int Time_EN = 500;
int Time_Go = 0;
int Time_D = 10;
u8 Key_Flag = 0;//按键返回1234

int Count_TIM4 ; //脉冲数取值
int Wei_Yi;       //更新脉冲数
int Wei_Yi_Last = 0;//上一次的位移数值
int Speed; //电机的速度
int Ju_Li = 0;//速度的积分

int Wei_Yi_flag=0;
int ZhouQi = 0;

 u8 i = 0,j = 0;	 
float Lvbo[N] = {0};
int Mode1_Flag = 0,Mode4_Flag = 0;

/***************DMA打印函数****************/
/****
  shu1:3位，shu2:5位，shu3:3位，shu4:4位，shu5:9位
*****/
void DMA_Send(int start,int Shu1,int Shu2 ,int Shu3,int Shu4,int Shu5)  //start：数组的起始位置 shu：变量
{   
	  if(Shu1 < 0)
		{
      Shu1 = -Shu1; 
			SendBuff[start]   = '-';
			SendBuff[start+1] = Shu1/100 + 0x30;
			SendBuff[start+2] = Shu1%100/10 + 0x30;
			SendBuff[start+3] = Shu1%10 + 0x30;
			SendBuff[start+4] = ' ';
    }
		else 
		{
      SendBuff[start]   = '+';
			SendBuff[start+1] = Shu1/100 + 0x30;
			SendBuff[start+2] = Shu1%100/10 + 0x30;
			SendBuff[start+3] = Shu1%10 + 0x30;
			SendBuff[start+4] = ' ';
    }
		if(Shu2<0)
		{
			Shu2 = -Shu2;
			SendBuff[start+5] = '-';
			SendBuff[start+6] = Shu2/10000 + 0x30;
			SendBuff[start+7] = Shu2%10000/1000 + 0x30;
			SendBuff[start+8] = Shu2%1000/100 + 0x30;
			SendBuff[start+9] = Shu2%100/10 + 0x30;
			SendBuff[start+10] = Shu2%10 + 0x30;
			SendBuff[start+11] = ' ';
		}
		else 
		{
			SendBuff[start+5] = '+';
      SendBuff[start+6] = Shu2/10000 + 0x30;
			SendBuff[start+7] = Shu2%10000/1000 + 0x30;
			SendBuff[start+8] = Shu2%1000/100 + 0x30;
			SendBuff[start+9] = Shu2%100/10 + 0x30;
			SendBuff[start+10] = Shu2%10 + 0x30;
			SendBuff[start+11] = ' ';
    }
	  
		if(Shu3 < 0)
		{
      Shu3 = -Shu3; 
			SendBuff[start+12]   = '-';
			SendBuff[start+13] = Shu3/100 + 0x30;
			SendBuff[start+14] = Shu3%100/10 + 0x30;
			SendBuff[start+15] = Shu3%10 + 0x30;
			SendBuff[start+16] = ' ';
    }
		else 
		{
      SendBuff[start+12]   = '+';
			SendBuff[start+13] = Shu3/100 + 0x30;
			SendBuff[start+14] = Shu3%100/10 + 0x30;
			SendBuff[start+15] = Shu3%10 + 0x30;
			SendBuff[start+16] = ' ';
    }
		
		if(Shu4 < 0)
		{
      Shu4 = -Shu4; 
			SendBuff[start+17]   = '-';
			SendBuff[start+18] = Shu4/1000 + 0x30;
			SendBuff[start+19] = Shu4%1000/100 + 0x30;
			SendBuff[start+20] = Shu4%100/10 + 0x30;
			SendBuff[start+21] = Shu4%10 + 0x30;
			SendBuff[start+22] = ' ';
    }
		else 
		{
		  SendBuff[start+17]   = '+';
			SendBuff[start+18] = Shu4/1000 + 0x30;
			SendBuff[start+19] = Shu4%1000/100 + 0x30;
			SendBuff[start+20] = Shu4%100/10 + 0x30;
			SendBuff[start+21] = Shu4%10 + 0x30;
			SendBuff[start+22] = ' ';
    }
		if(Shu5 < 0)
		{
      Shu5 = -Shu5; 
			SendBuff[start+23]   = '-';
			SendBuff[start+24] = Shu5/100000000 + 0x30;
			SendBuff[start+25] = Shu5%100000000/10000000 + 0x30;
			SendBuff[start+26] = Shu5%10000000/1000000 + 0x30;
			SendBuff[start+27] = Shu5%1000000/100000 + 0x30;
			SendBuff[start+28] = Shu5%100000/10000 + 0x30;
			SendBuff[start+29] = Shu5%10000/1000 + 0x30;
			SendBuff[start+30] = Shu5%1000/100 + 0x30;
			SendBuff[start+31] = Shu5%100/10 + 0x30;
			SendBuff[start+32] = Shu5%10 + 0x30;
			SendBuff[start+33] = ' ';
    }
		else 
		{
		  SendBuff[start+23]   = '+';
			SendBuff[start+24] = Shu5/100000000 + 0x30;
			SendBuff[start+25] = Shu5%100000000/10000000 + 0x30;
			SendBuff[start+26] = Shu5%10000000/1000000 + 0x30;
			SendBuff[start+27] = Shu5%1000000/100000 + 0x30;
			SendBuff[start+28] = Shu5%100000/10000 + 0x30;
			SendBuff[start+29] = Shu5%10000/1000 + 0x30;
			SendBuff[start+30] = Shu5%1000/100 + 0x30;
			SendBuff[start+31] = Shu5%100/10 + 0x30;
			SendBuff[start+32] = Shu5%10 + 0x30;
			SendBuff[start+33] = ' ';
    }
		
		SendBuff[start+34]= '\n';
}

/*************电机控制：正转***************/
void Motor_Zheng(int pwm)
{
  TIM_SetCompare1(TIM3,pwm);
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}

/*************电机控制：反转**************/
void Motor_Fan(int pwm)
{
  TIM_SetCompare1(TIM3,1000-pwm);
	GPIO_SetBits(GPIOC, GPIO_Pin_4);
}



 int main(void)
 {
//   u8 i = 0,j = 0;	 
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为9600
	 
	printf("***********************\r\n");
	USART1_DMA_Config(); 
	LED_GPIO_Config();   //LED初始化 ：PC15
  TIM2_Init();  //定时器2初始化
	TIM3_PWM_Init();//定时器3PWM输出初始化
	Time4_Config(); //TIM4编码器模式
	ADC1_Init();  //AD初始化 PC0 
	/*****************按键初始化****************/ 
	KEY_Init();//IO初始化	
  EXTIX_Init();//外部中断初始化	
  
	printf("111111111111111111111111\r\n");
	
  printf("333333333333333333333333333\r\n");

  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);  //串口1的DMA使能

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE); //开启定时器TIM2
	
	TIM_SetCounter(TIM4, 5050);  //TIM4编码器计数初始值
	
	while(1)
	{
		Count_TIM4 = (TIM4 -> CNT) - 5050;   //定时器4脉冲计数，编码器输入，初始为5500
		
		Wei_Yi = 5000*ZhouQi + Count_TIM4;   //就算出位移
		
		
		if((TIM4 -> CNT) >= 10050)
		{
       (TIM4 -> CNT) = 5050;
			 ZhouQi ++;             //转的圈数++
    }
		else if((TIM4 -> CNT) <= 50)
		{
       (TIM4 -> CNT) = 5050;
			  ZhouQi--;                  //转的圈数--
    } 
		
		/********递推均值滤波********/
		Lvbo[i++] =(ADC_ConvertedValue - 2048)*0.0879 + err; // 读取转换的AD值(int)
		if(i == N)
		{
			i = 0;
		}
		
		for(j = 0;j<N;j++)
		{
			ADC_ConvertedValueLocal = ADC_ConvertedValueLocal + Lvbo[j];
		}
		
		/*****按键4按下后给偏移角Go,其他状态不要偏移*********/
		if(Key_Flag != 4)
		{
		 Angle = ADC_ConvertedValueLocal /10;
		}
		else
		{
       Angle = ADC_ConvertedValueLocal /10+Go;
    }

		ADC_ConvertedValueLocal  = 0;
    
		/**********状态0关闭电机*******/
		if(Key_Flag ==0)
		{
      	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
    }
		else if(Key_Flag == 1)       //按键1，基本题第一二题，完成圆周运动
		{   
			  GPIO_SetBits(GPIOC, GPIO_Pin_5);
				switch(Mode1_Flag)
				{
					case 0:{Motor_Zheng(800);}break;
					case 1:{Motor_Fan(900);}break;
					case 2:{Motor_Zheng(950);}break;
					case 3:{Motor_Fan(950);}break;
					case 4:{Motor_Zheng(800);}break;
					case 5:{Motor_Zheng(0);Key_Flag =0;}break;
				}
	  }
		
		
		else if(Key_Flag == 3)    //按键3，摆摆动至+-35度进入状态6，进行PID调节
		{
        GPIO_SetBits(GPIOC, GPIO_Pin_5);
				switch(Mode1_Flag)
				{
					case 0:{Motor_Zheng(800);}break;
					case 1:{Motor_Fan(900);}break;
					case 2:{Motor_Zheng(180);}break; //200
					case 3:{
						       Motor_Zheng(0);
						       if(Angle>-35&& Angle<35)
                   {
                    Key_Flag  = 6;
                   } 
									 else 
									 {
                    Key_Flag =0;
                   }
                  }break;
				}
    }
	  LED1_TOGGLE	;
 		DMA_Send(0,Angle,Count_TIM4,Angle_err,Speed,Ju_Li);//Angle_jifen
		LED1_TOGGLE	;

	}
 }

 /*******************中断函数*******************/
 void TIM2_IRQHandler(void)
{  
		if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
		{	
			time--;	
		
			if(time <= 0)
			{
				time = 2;
				if(Key_Flag == 2)   //状态2的PID控制
				{
							Angle_err = Angle-Angle_Last;
						
							Angle_Last = Angle;
							
							Speed = Wei_Yi - Wei_Yi_Last;
							
							Wei_Yi_Last = Wei_Yi;
							
							Ju_Li += Speed;
							if(Ju_Li > Ju_Li_Max)
							{
                 Ju_Li = Ju_Li_Max ;
              }
							if(Ju_Li < -Ju_Li_Max)
							{
                 Ju_Li = -Ju_Li_Max ;
              }
					
							
						PWM_WEIYI = -Speed*Kp_b -Ju_Li*Ki_b ;				
 					  
						PWM_ANGLE = (Angle)*Kp + Angle_err*Kd;
						
						PWM_OUT = PWM_ANGLE + PWM_WEIYI;
 					
 					  if(PWM_OUT < 0)
						{
							if(PWM_OUT<=-1000)
							{
								PWM_OUT=-1000;
							}
							Motor_Zheng(-PWM_OUT);
						}
						else
						{
							if(PWM_OUT>=1000)
							{
								PWM_OUT=1000;
							}
							 Motor_Fan(PWM_OUT);
						}
						if(Angle >= 45 || Angle <=-45)
						{
						  GPIO_ResetBits(GPIOC, GPIO_Pin_5);
              Motor_Fan(0);
							Key_Flag = 0;
							PWM_OUT = 0;
							Angle_jifen=0;
						  Wei_Yi_flag=0;
            }
			  }
				
				if(Key_Flag == 6)   //状态6的PID控制，和状态2一样，可以适当微调
				{
						Angle_err = Angle-Angle_Last;
						
							Angle_Last = Angle;
							
							Speed = Wei_Yi - Wei_Yi_Last;
							
							Wei_Yi_Last = Wei_Yi;
							
							Ju_Li += Speed;
							if(Ju_Li > Ju_Li_Max)
							{
                 Ju_Li = Ju_Li_Max ;
              }
							if(Ju_Li < -Ju_Li_Max)
							{
                 Ju_Li = -Ju_Li_Max ;
              }
					
							
						PWM_WEIYI = -Speed*Kp_b -Ju_Li*Ki_b ;				
 					  
						PWM_ANGLE = (Angle)*Kp + Angle_err*Kd;
						
						PWM_OUT = PWM_ANGLE + PWM_WEIYI;
 					
 					  if(PWM_OUT < 0)
						{
							if(PWM_OUT<=-1000)
							{
								PWM_OUT=-1000;
							}
							Motor_Zheng(-PWM_OUT);
						}
						else
						{
							if(PWM_OUT>=1000)
							{
								PWM_OUT=1000;
							}
							 Motor_Fan(PWM_OUT);
						}
						if(Angle >= 45 || Angle <=-45)
						{
						  GPIO_ResetBits(GPIOC, GPIO_Pin_5);
              Motor_Fan(0);
							Key_Flag = 0;
							PWM_OUT = 0;
							Angle_jifen=0;
						  Wei_Yi_flag=0;
            }
			  }
				
				
				if(Key_Flag == 4)   //状态4：给一个偏移角后进行同样的PID控制，系统会往偏移方向摆动
				{
           Angle_err = Angle-Angle_Last;
						
							Angle_Last = Angle;
							
							Speed = Wei_Yi - Wei_Yi_Last;
							
							Wei_Yi_Last = Wei_Yi;
							
							Ju_Li += Speed;
							if(Ju_Li > Ju_Li_Max)
							{
                 Ju_Li = Ju_Li_Max ;
              }
							if(Ju_Li < -Ju_Li_Max)
							{
                 Ju_Li = -Ju_Li_Max ;
              }
					
							
						PWM_WEIYI = -Speed*Kp_b -Ju_Li*Ki_b ;				
 					  
						PWM_ANGLE = (Angle)*Kp + Angle_err*Kd;
						
						PWM_OUT = PWM_ANGLE + PWM_WEIYI;
 					
 					  if(PWM_OUT < 0)
						{
							if(PWM_OUT<=-1000)
							{
								PWM_OUT=-1000;
							}
							Motor_Zheng(-PWM_OUT);
						}
						else
						{
							if(PWM_OUT>=1000)
							{
								PWM_OUT=1000;
							}
							 Motor_Fan(PWM_OUT);
						}
						if(Angle >= 45 || Angle <=-45)
						{
						  GPIO_ResetBits(GPIOC, GPIO_Pin_5);
              Motor_Fan(0);
							Key_Flag = 0;
							PWM_OUT = 0;
							Angle_jifen=0;
						  Wei_Yi_flag=0;
            }
			  }
				
			}		

			if(Key_Flag == 1)   //按键1按下后，控制摆动的时间
			{
         Time_EN --;
				if(Time_EN <=0 && Mode1_Flag <5)
			 {
         Time_EN =500;
				 Mode1_Flag ++;
				 if(Mode1_Flag >= 5)
				 {
            Mode1_Flag = 5;
         }
       }
      }
			
		  if(Key_Flag == 3)
			{
        Time_EN --;
			 if(Time_EN <=0 && Mode1_Flag <3)
			 {
         Time_EN =500;
				 Mode1_Flag ++;
				 if(Mode1_Flag >= 3)
				 {
            Mode1_Flag = 3;
         }
       }
      }
			
			if(Key_Flag == 4)
			{
        Time_Go--;
				if(Time_Go <= 0)
				{
					 Time_Go = 0;
					 Go = 0;
					 Key_Flag = 2;
        }
      }
			TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);  
		}		 	
}



//外部中断0服务程序 
void EXTI1_IRQHandler(void)
{
	delay_ms(10);//消抖
	
	if(KEY3==1)	 	 //WK_UP按键1
	{				 
	 	Key_Flag = 1;
		Time_EN = 500;
		 Mode1_Flag = 0;
	}
	EXTI_ClearITPendingBit(EXTI_Line1); //清除LINE0上的中断标志位  
}
 

//外部中断2服务程序
void EXTI2_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY2==0)	  //按键KEY2  
	{
    Kp = 160;//200
    Kd = 1.5; //10
		Kp_b = 0.2;//5 0.475
		Ki_b = 2;
		Ju_Li_Max=200;
		Speed=0;
		Key_Flag = 2;
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		Ju_Li = 0;
  }		 
	EXTI_ClearITPendingBit(EXTI_Line2);  //清除LINE2上的中断标志位  
}

//外部中断3服务程序
void EXTI3_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY1==0)	 //按键KEY3
	{				 
	  Kp = 160;//200
    Kd = 1.5; //10
		Kp_b = 0.2;//5 0.475
		Ki_b = 2;
		Ju_Li_Max=200;
		Speed=0;
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
	  Key_Flag = 3;
		Time_EN = 500;
		Mode1_Flag = 0;
		Ju_Li = 0;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
}


void EXTI4_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY0==0)	 //按键KEY4
	{
		Kp = 160;//200
    Kd = 1.5; //10
		Kp_b = 0.2;//5 0.475
		Ki_b = 2;
		Ju_Li_Max=200;
		Speed=0;
		Key_Flag = 4;
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		Ju_Li = 0;
		Time_Go = 3500;
		Go = 3.0;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
}
