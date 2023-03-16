#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/stm32l051xx.h"

#define F_CPU 32000000L

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
//       PA5 -|11      22|- PA12
//       PA6 -|12      21|- PA11
//       PA7 -|13      20|- PA10 (Reserved for RXD)
//       PB0 -|14      19|- PA9  (Reserved for TXD)
//       PB1 -|15      18|- PA8  (Measure the period at this pin)
//       VSS -|16      17|- VDD
//             ----------

void wait_1ms(void)
{
	// For SysTick info check the STM32L0xxx Cortex-M0 programming manual page 85.
	SysTick->LOAD = (F_CPU/1000L) - 1;  // set reload register, counter rolls over from zero, hence -1
	SysTick->VAL = 0; // load the SysTick counter
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick IRQ and SysTick Timer */
	while((SysTick->CTRL & BIT16)==0); // Bit 16 is the COUNTFLAG.  True when counter rolls over from zero.
	SysTick->CTRL = 0x00; // Disable Systick counter
}

void waitms(int len)
{
	while(len--) wait_1ms();
}

void main(void)
{
	int current, previous;
	
	RCC->IOPENR |= 0x00000001; // peripheral clock enable for port A
	
	GPIOA->MODER &= ~(BIT22 | BIT23); // Make pin PA11 input
	// Activate pull up for pin PA11:
	GPIOA->PUPDR |= BIT22; 
	GPIOA->PUPDR &= ~(BIT23); 
	
	waitms(500); // Give putty a chance to start before sending info
	printf("Push button test for the STM32L051.\r\nConnect push button between PA11 (pin 21) and ground.\r\n\r\n");
	previous=(GPIOA->IDR&BIT11)?0:1;
	while (1)
	{
		current=(GPIOA->IDR&BIT11)?1:0;
		if(current!=previous)
		{
			previous=current;
			printf("PA11=%d\r", current);
			fflush(stdout);
		}
	}
}
