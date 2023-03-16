// This program activates the PWM function of timer 2 channel 1 on PA15 (pin 25).
// The PWM is changed every second or so using the ISR for TIME2.
// An LED+1k could be attached to PA0 (pin 6).  Every time the LED toggles, the PWM changes.

#include "../Common/Include/stm32l051xx.h"

volatile int Count = 0;

#define SYSCLK 32000000L
#define TICK_FREQ 1000L

void ToggleLED(void) 
{    
	GPIOA->ODR ^= BIT0; // Toggle PA0
}

// Interrupt service routines are the same as normal
// subroutines (or C funtions) in Cortex-M microcontrollers.
// The following should happen at a rate of 1kHz.
// The following function is associated with the TIM2 interrupt 
// via the interrupt vector table defined in startup.c
void TIM2_Handler(void) 
{
	TIM2->SR &= ~BIT0; // clear update interrupt flag
	Count++;
	if (Count > 60000)
	{ 
		TIM2->CCR1=(TIM2->CCR1+16)&0xff;
		Count = 0;
		ToggleLED(); // toggle the state of the LED 
	}   
}

// LQFP32 pinout
//             ----------
//       VDD -|1       32|- VSS
//      PC14 -|2       31|- BOOT0
//      PC15 -|3       30|- PB7
//      NRST -|4       29|- PB6
//      VDDA -|5       28|- PB5
// (LED) PA0 -|6       27|- PB4
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
	// Set up output port bit for blinking LED
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
    GPIOA->MODER = (GPIOA->MODER & ~(BIT1)) | BIT0; // Make pin PA0 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)

	// Configure PA15 for altenate function (TIM2_CH1, pin 25 in LQFP32 package)
	GPIOA->OSPEEDR  |= BIT30; // MEDIUM SPEED
	GPIOA->OTYPER   &= ~BIT15; // Push-pull
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT30)) | BIT31; // AF-Mode
	GPIOA->AFR[1]   |= BIT30 | BIT28 ; // AF5 selected (check table 16 in page 43 of "en.DM00108219.pdf")
	
	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	//TIM2->ARR = SYSCLK/TICK_FREQ;
	TIM2->ARR = 255;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	// Enable PWM in channel 1 of Timer 2
	TIM2->CCMR1|=BIT6|BIT5; // PWM mode 1 ([6..4]=110)
	TIM2->CCMR1|=BIT3; // OC1PE=1
	TIM2->CCER|=BIT0; // Bit 0 CC1E: Capture/Compare 1 output enable.
	
	// Set PWM to 50%
	//TIM2->CCR1=SYSCLK/(TICK_FREQ*2);
	TIM2->CCR1=128;
	TIM2->EGR |= BIT0; // UG=1
	
	__enable_irq();
}

int main(void)
{
	Hardware_Init();
	while(1)
	{
	}
	return 0;
}
