#ifndef MACROS_H
#define MACROS_H

#define F_CPU 32000000L
#define SYSCLK 32000000L
#define DEF_F 100000L // 10us tick

// Some 'defines' to turn pins on/off easily (pins must be configured as outputs)
#define PB3_0 (GPIOB->ODR &= ~BIT3)
#define PB3_1 (GPIOB->ODR |=  BIT3)
#define PB4_0 (GPIOB->ODR &= ~BIT4)
#define PB4_1 (GPIOB->ODR |=  BIT4)
#define PB5_0 (GPIOB->ODR &= ~BIT5)
#define PB5_1 (GPIOB->ODR |=  BIT5)
#define PB6_0 (GPIOB->ODR &= ~BIT6)
#define PB6_1 (GPIOB->ODR |=  BIT6)
#define PB7_0 (GPIOB->ODR &= ~BIT7)
#define PB7_1 (GPIOB->ODR |=  BIT7)

extern volatile int PWM_Counter;
extern volatile unsigned char ISR_pwm1, ISR_pwm2;

void tone(unsigned int frequency, unsigned int duration);

extern volatile unsigned char mode;
void togglemode(void);

void wait_1ms(void);
void TIM2_Handler(void);

// A define to easily read PA8 (PA8 must be configured as input first)
#define PA8 (GPIOA->IDR & BIT8)
long int GetPeriod (int n);

void PrintNumber(long int val, int Base, int digits);

// for timer21
#define TICK_FREQ 1000L
extern volatile int OffCycles;

#endif