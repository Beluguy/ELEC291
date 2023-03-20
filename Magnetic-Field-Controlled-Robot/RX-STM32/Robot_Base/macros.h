#ifndef MACROS_H
#define MACROS_H

#define F_CPU 32000000L
#define DEF_F 100000L // 10us tick

extern volatile int PWM_Counter = 0;
extern volatile unsigned char ISR_pwm1=100, ISR_pwm2=100;

void wait_1ms(void);
void waitms(int len);
void TIM2_Handler(void);

// A define to easily read PA8 (PA8 must be configured as input first)
#define PA8 (GPIOA->IDR & BIT8)
long int GetPeriod (int n);

void PrintNumber(long int val, int Base, int digits);

#endif