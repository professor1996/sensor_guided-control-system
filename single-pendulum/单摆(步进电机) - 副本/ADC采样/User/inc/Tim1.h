#include "stm32f10x.h"
#include "adc.h"
#include "IMU.h"


extern float Rate_rod;

void TIM1_Configuration(void);
void TIM1_Dis(void);
void TIM1_Start(void);
float Kalman_Filter(float ResrcData,float ProcessNiose_Q,float MeasureNoise_R);




