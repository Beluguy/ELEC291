#include "../Common/Include/stm32l051xx.h"
#include "serial.h"

void initUART(int BaudRate)
{
	int BaudRateDivisor;

	// Turn on the clock for GPIOA (usart 1 uses it)
	RCC->IOPENR |= BIT0;
	
	BaudRateDivisor = 32000000; // assuming 32MHz clock 
	BaudRateDivisor = BaudRateDivisor / (long) BaudRate;

	//Configure PA9 (TXD for USART1, pin 19 in LQFP32 package)
	GPIOA->OSPEEDR  |= BIT18; // MEDIUM SPEED
	GPIOA->OTYPER   &= ~BIT9; // Push-pull
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT18)) | BIT19; // AF-Mode
	GPIOA->AFR[1]   |= BIT6 ; // AF4 selected
	
	//Configure PA10 (RXD for USART1, pin 20 in LQFP32 package)
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT20)) | BIT21; // AF-Mode
	GPIOA->AFR[1]   |= BIT10;  // AF4 selected
	
	RCC->APB2ENR    |= BIT14; // Turn on the clock for the USART1 peripheral

	USART1->CR1 |= (BIT2 | BIT3 | BIT5 | BIT6); // Enable Transmitter, Receiver, Transmit & Receive interrupts.
	USART1->CR2 = 0x00000000;
	USART1->CR3 = 0x00000000;           
	USART1->BRR = BaudRateDivisor;
	USART1->CR1 |= BIT0; // Enable Usart 1
}

void uart_putc(unsigned char c)
{
	USART1->TDR=c;
	while((USART1->ISR&BIT6)==0);  // Wait for transmission complete
}

unsigned char uart_getc(void)
{
    unsigned char c;
    
    while((USART1->ISR&BIT5)==0);  // Wait for reception complete
    c=USART1->RDR;
    return (c);
}

int eputs(char *s)
{
	while (*s) uart_putc(*s++);
	return 0;
}

void eputc(char c)
{
	uart_putc(c);
}

char egetc(void)
{
	return uart_getc();
}

int egets(char *s, int Max)
{
	int Len;
	char c;
	
	Len=0; // Until initialization code is put in init.s, we have to do this
	c = 0;
	while ( (Len < Max-1) && (c != NEWLINE) )
	{   
		c = uart_getc();
		s[Len++] = c;
	}
	if (Len>0)
	{
		s[Len]=0;
	}	
	return Len;
}
