#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"

#define F_CPU 32000000L
#define DEF_F 100000L // 10us tick

volatile int PWM_Counter = 0;
volatile unsigned char pwm1=100, pwm2=100;

void wait_1ms(void)
{
	// For SysTick info check the STM32l0xxx Cortex-M0 programming manual.
	SysTick->LOAD = (F_CPU/1000L) - 1;  // set reload register, counter rolls over from zero, hence -1
	SysTick->VAL = 0; // load the SysTick counter
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick IRQ and SysTick Timer */
	while((SysTick->CTRL & BIT16)==0); // Bit 16 is the COUNTFLAG.  True when counter rolls over from zero.
	SysTick->CTRL = 0x00; // Disable Systick counter
}

void delayms(int len)
{
	while(len--) wait_1ms();
}

// Interrupt service routines are the same as normal
// subroutines (or C funtions) in Cortex-M microcontrollers.
// The following should happen at a rate of 1kHz.
// The following function is associated with the TIM2 interrupt 
// via the interrupt vector table defined in startup.c
void TIM2_Handler(void) 
{
	TIM2->SR &= ~BIT0; // clear update interrupt flag
	PWM_Counter++;
	
	if(pwm1>PWM_Counter)
	{
		GPIOA->ODR |= BIT11;
	}
	else
	{
		GPIOA->ODR &= ~BIT11;
	}
	
	if(pwm2>PWM_Counter)
	{
		GPIOA->ODR |= BIT12;
	}
	else
	{
		GPIOA->ODR &= ~BIT12;
	}
	
	if (PWM_Counter > 2000) // THe period is 20ms
	{
		PWM_Counter=0;
		GPIOA->ODR |= (BIT11|BIT12);
	}   
}

// LQFP32 pinout
//             ----------
//       VDD -|1       32|- VSS
//      PC14 -|2       31|- BOOT0
//      PC15 -|3       30|- PB7
//      NRST -|4       29|- PB6
//      VDDA -|5       28|- PB5
//       PA0 -|6       27|- PB4
//       PA1 -|7       26|- PB3
//       PA2 -|8       25|- PA15
//       PA3 -|9       24|- PA14
//       PA4 -|10      23|- PA13
//       PA5 -|11      22|- PA12 (pwm2)
//       PA6 -|12      21|- PA11 (pwm1)
//       PA7 -|13      20|- PA10 (Reserved for RXD)
//       PB0 -|14      19|- PA9  (Reserved for TXD)
//       PB1 -|15      18|- PA8
//       VSS -|16      17|- VDD
//             ----------

void Hardware_Init(void)
{
	// Set up output pins
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
    GPIOA->MODER = (GPIOA->MODER & ~(BIT22|BIT23)) | BIT22; // Make pin PA11 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	GPIOA->OTYPER &= ~BIT11; // Push-pull
    GPIOA->MODER = (GPIOA->MODER & ~(BIT24|BIT25)) | BIT24; // Make pin PA12 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	GPIOA->OTYPER &= ~BIT12; // Push-pull

	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = F_CPU/DEF_F-1;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	__enable_irq();
}

int main(void)
{
    char buf[32];
    int npwm;

	Hardware_Init();
	
	delayms(500); // Give putty a chance to start before we send characters with printf()
	
    printf("Servo signal generatioin using the STM32L051 using TIM2\r\n");
    printf("(outputs are PA11 and PA12, pins 21 and 22).\r\n");
    printf("By Jesus Calvino-Fraga (c) 2018-2023.\r\n\r\n");
	
	while (1)
	{
    	printf("PWM1 (60 to 255): ");
    	fflush(stdout);
    	egets_echo(buf, 31); // wait here until data is received
  		printf("\r\n");
	    npwm=atoi(buf);

	    if(npwm!=0)
	    {
		    if(npwm>255) npwm=255;
		    if(npwm<60) npwm=60;
		    pwm1=npwm;
	    }
	    
    	printf("PWM2 (60 to 255): ");
    	fflush(stdout);
    	egets_echo(buf, 31); // wait here until data is received
 		printf("\r\n");
 
	    npwm=atoi(buf);
	    if(npwm!=0)
	    {
		    if(npwm>255) npwm=255;
		    if(npwm<60) npwm=60;
		    pwm2=npwm;
		}
	}
}
