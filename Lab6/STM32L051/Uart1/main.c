// Warning: not the best project to base anything into it.  I just used it to
// test USART1.  You'll be better off using any of the other projects where
// the startup code is written in 'c', unlike this one where the startup is
// written in assembly. On the plus side, the generated binary is tiny!

#include <stdio.h>
#include "../Common/Include/stm32l051xx.h"

#define SystemCoreClock 2097000L // This is the clock after reset
#define baud 115200L

// delay loop for the default 2.1 MHz CPU clock with optimizer enabled
void delay(uint32_t msec)
{
    for (uint32_t j=0; j < 419UL * msec; j++)
    {
        __NOP();
    }
}

// Redirect standard output to the serial port
int _write(int file, char *ptr, int len)
{
    for (int i=0; i<len; i++)
    {
        while(!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *ptr++;
    }
    return len;
}

void myputs (char * s)
{
	while(*s)
	{
        while(!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *s++;
	}
}

// Called after each received character
void USART1_IRQHandler()
{
    // read the received character
    char received=USART1->RDR;

    // send echo back
    while(!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = received;
}

// http://stefanfrings.de/stm32/stm32l0.html
int main()
{
    // Enable clock for Port A
    RCC->IOPENR|=RCC_IOPENR_GPIOAEN;

    // PA6 = Output for the LED
    GPIOA->MODER = (GPIOA->MODER & 0xfffffffc) | 0x00000001; // Make pin PA0 output (page 172, two bits used to configure: bit0=1, bit1=0)

    // Use system clock for USART1
    RCC->APB2ENR|=RCC_APB2ENR_USART1EN;
    RCC->CCIPR = (RCC->CCIPR & ~(BIT11)) | BIT10;

    // PA9 (TxD) shall use the alternate function 4 (see data sheet)
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT18)) | BIT19; // AF-Mode
	GPIOA->AFR[1]   |= BIT6 ; // AF4 selected

    // PA10 (RxD) shall use the alternate function 4 (see data sheet)
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT20)) | BIT21; // AF-Mode
	GPIOA->AFR[1]   |= BIT10;  // AF4 selected

    // Set baudrate
    USART1->BRR = SystemCoreClock / baud;

    // Enable transmitter, receiver and receive-interrupt of USART1
    USART1->CR1 = USART_CR1_UE + USART_CR1_TE + USART_CR1_RE + USART_CR1_RXNEIE;

    // Enable interrupt in NVIC
	NVIC->ISER[0] |= BIT27; // Enable USART1 interrupts in NVIC	
	__enable_irq();

    while (1)
    {
		GPIOA->ODR ^= 0x00000001; // Toggle PA0
        delay(100);

        myputs("Hello there!\r\n");
    }
}