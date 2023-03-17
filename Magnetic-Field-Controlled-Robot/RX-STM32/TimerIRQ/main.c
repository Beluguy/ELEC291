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
// via the interrupt vector table defined in startup.s
void TIM2_Handler(void) 
{
	TIM2->SR &= ~BIT0; // clear update interrupt flag
	Count++;
	if (Count > 500)
	{ 
		Count = 0;
		ToggleLED(); // toggle the state of the LED every half second
	}   
}

void Hardware_Init(void)
{
	// Set up output port bit for blinking LED
	RCC->IOPENR |= 0x00000001; // peripheral clock enable for port A
    GPIOA->MODER |= (GPIOA->MODER & 0xfffffffc) | 0x00000001; // Make pin PA0 output (page 172, two bits used to configure: bit0=1, bit1=0)
	
	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = SYSCLK/TICK_FREQ;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT0;      // enable counting    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt  
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
