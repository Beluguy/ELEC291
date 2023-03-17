#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"

#define SYSCLK 32000000L
#define DEF_F 15000L

// LQFP32 pinout
//             ----------
//       VDD -|1       32|- VSS
//      PC14 -|2       31|- BOOT0
//      PC15 -|3       30|- PB7
//      NRST -|4       29|- PB6
//      VDDA -|5       28|- PB5
//       PA0 -|6       27|- PB4
//       PA1 -|7       26|- PB3
//       PA2 -|8       25|- PA15 (PWM output channel 1 of TIM2)
//       PA3 -|9       24|- PA14
//       PA4 -|10      23|- PA13
//       PA5 -|11      22|- PA12
//       PA6 -|12      21|- PA11
//       PA7 -|13      20|- PA10 (Reserved for RXD)
//       PB0 -|14      19|- PA9  (Reserved for TXD)
//       PB1 -|15      18|- PA8
//       VSS -|16      17|- VDD
//             ----------

void Hardware_Init(void)
{
	GPIOA->OSPEEDR=0xffffffff; // All pins of port A configured for very high speed! Page 201 of RM0451

	// Configure PA15 for altenate function (TIM2_CH1, pin 25 in LQFP32 package)
	GPIOA->OSPEEDR  |= BIT30; // MEDIUM SPEED
	GPIOA->OTYPER   &= ~BIT15; // Push-pull
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT30)) | BIT31; // AF-Mode
	GPIOA->AFR[1]   |= BIT30 | BIT28 ; // AF5 selected (check table 16 in page 43 of "en.DM00108219.pdf")
	
	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = SYSCLK/DEF_F-1;
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	// Enable PWM in channel 1 of Timer 2
	TIM2->CCMR1|=BIT6|BIT5; // PWM mode 1 ([6..4]=110)
	TIM2->CCMR1|=BIT3; // OC1PE=1
	TIM2->CCER|=BIT0; // Bit 0 CC1E: Capture/Compare 1 output enable.
	
	// Set PWM to 50%
	TIM2->CCR1=(SYSCLK/(DEF_F*2));
	TIM2->EGR |= BIT0; // UG=1
	
}

int main(void)
{
    char buf[32];
    int newF, reload;

	Hardware_Init();
	
    printf("Frequency generator for the STM32L051 using TIM2 (output is in PA15, pin 25).\r\n");
    printf("By Jesus Calvino-Fraga (c) 2018-2023.\r\n\r\n");
	
	while (1)
	{
    	printf("Frequency: ");
    	fflush(stdout);
    	egets_echo(buf, 31); // wait here until data is received
 
	    newF=atoi(buf);
	    if(newF>(SYSCLK/4))
	    {
	       printf("\r\nWarning: The maximum frequency is SYSCLK/4.\r\n"
	             "Setting frequency to SYSCLK/4.\r\n");
	       newF=SYSCLK/4;
	    }
	    if(newF>0)
	    {
		    reload=(SYSCLK/newF);
		    printf("\r\nFrequency set to: %d\r\n", SYSCLK/reload);
			TIM2->CR1 &= ~BIT0; // disable timer    
			TIM2->ARR = reload-1;
			TIM2->CCR1= reload/2; // Adjust PWM output to 50%
			TIM2->EGR |= BIT0; // UG=1
			TIM2->CR1 |= BIT0; // enable timer    
        }
	}
}
