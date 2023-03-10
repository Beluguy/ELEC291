#include "../Include/stm32l051xx.h"
#include "../Include/serial.h"

// Serial comms routine for the stm32l051 microcontroller.
// Makes use of USART1.  Pins PA9 and PA10 are used for transmission/reception.
// Defines a new version of puts: e(mbedded)puts and egets
// Similar to puts and gets in standard C however egets checks the size
// of the input buffer.  This could be extended to include a timeout quite easily.
// Written by Frank Duignan and modified by Jesus Calvino-Fraga
// 

// define the size of the communications buffer (adjust to suit)
#define MAXBUFFER   64
typedef struct tagComBuffer{
    unsigned char Buffer[MAXBUFFER];
    unsigned Head,Tail;
    unsigned Count;
} ComBuffer;

ComBuffer ComRXBuffer, ComTXBuffer;

int PutBuf(ComBuffer  *Buf,unsigned char Data);
unsigned char GetBuf(ComBuffer  *Buf);
unsigned GetBufCount(ComBuffer  *Buf);
int ReadCom(int Max,unsigned char *Buffer);
int WriteCom(int Count,unsigned char *Buffer);

void usart_tx (void);
void usart_rx (void);
unsigned ComOpen;
unsigned ComError;
unsigned ComBusy;

int ReadCom(int Max,unsigned char *Buffer)
{
	// Read up to Max bytes from the communications buffer
	// into Buffer.  Return number of bytes read
	unsigned i;
  	if (!ComOpen)
    	return (-1);
	i=0;
	while ((i < Max-1) && (GetBufCount(&ComRXBuffer)))
		Buffer[i++] = GetBuf(&ComRXBuffer);
	if (i>0)
	{
		Buffer[i]=0;
		return(i);
	}
	else {
		return(0);
	}	
}

int WriteCom(int Count, unsigned char *Buffer)
{
	// Writes Count bytes from Buffer into the the communications TX buffer
	// returns -1 if the port is not open (configured)
	// returns -2 if the message is too big to be sent
	// If the transmitter is idle it will initiate interrupts by 
	// writing the first character to the hardware transmit buffer
	unsigned i;
	if (!ComOpen) return (-1);
		
	// Wait for transmission to complete before sending more
	if(Count<MAXBUFFER)
	{
		while ( (MAXBUFFER - GetBufCount(&ComTXBuffer)) < Count );
	}
	else
	{
		return (-2);
	}
		
	for(i=0; i<Count; i++) PutBuf(&ComTXBuffer,Buffer[i]);
	
	if ( (USART1->CR1 & BIT3)==0) // transmitter was idle, turn it on and force out first character
	{ 
		USART1->CR1 |= BIT3;
		USART1->TDR = GetBuf(&ComTXBuffer);		
	} 
	return 0;
}

void initUART(int BaudRate)
{
	int BaudRateDivisor;
	//int j;
	
	 __disable_irq();
	ComRXBuffer.Head = ComRXBuffer.Tail = ComRXBuffer.Count = 0;
	ComTXBuffer.Head = ComTXBuffer.Tail = ComTXBuffer.Count = 0;
	ComOpen = 1;
	ComError = 0;
	
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
	
	/* Test to see if baud rate is configured correctly.  Mesure the time of one bit.
	   The inverse of that time is the baud rate.*/
	//for(j=0; j<1000; j++)
	//{
	//    USART1->ISR &= ~BIT7;
	//	USART1->TDR = 'U';
	//  	while (	(USART1->ISR & BIT7)==0);
	//} 

	NVIC->ISER[0] |= BIT27; // Enable USART1 interrupts in NVIC	
	__enable_irq();
}

void USART1_Handler(void) 
{
	// check which interrupt happened.
    if (USART1->ISR & BIT7) // is it a TXE interrupt?
		usart_tx();
	if (USART1->ISR & BIT5) // is it an RXNE interrupt?
		usart_rx();
}

void usart_rx (void)
{
	// Handles serial comms reception
	// simply puts the data into the buffer and sets the ComError flag
	// if the buffer is fullup
	if ( PutBuf(&ComRXBuffer, USART1->RDR) )
	{
		ComError = 1; // if PutBuf returns a non-zero value then there is an error
	}
}

void usart_tx (void)
{
	// Handles serial comms transmission
	// When the transmitter is idle, this is called and the next byte
	// is sent (if there is one)
	if (GetBufCount(&ComTXBuffer)) USART1->TDR=GetBuf(&ComTXBuffer);
	else
	{
		// No more data, disable the transmitter 
		USART1->CR1 &= ~BIT3;
		if (USART1->ISR & BIT6) USART1->ICR |= BIT6; // Write TCCF to USART_ICR
		if (USART1->ISR & BIT7) USART1->RQR |= BIT4; // Write TXFRQ to USART_RQR
	}
}

int PutBuf(ComBuffer *Buf, unsigned char Data)
{
	//while ( (Buf->Head==Buf->Tail) && (Buf->Count!=0));  /* OverFlow? Wait... */
	if ( (Buf->Head==Buf->Tail) && (Buf->Count!=0)) return(1);  /* OverFlow */
	__disable_irq();
	Buf->Buffer[Buf->Head++] = Data;
	Buf->Count++;
	if (Buf->Head==MAXBUFFER) Buf->Head=0;
	__enable_irq();
	return(0);
}

unsigned char GetBuf(ComBuffer *Buf)
{
    unsigned char Data;
    if ( Buf->Count==0 ) return (0);
    __disable_irq();
    Data = Buf->Buffer[Buf->Tail++];
    if (Buf->Tail == MAXBUFFER) Buf->Tail = 0;
    Buf->Count--;
    __enable_irq();
    return (Data);
}

unsigned int GetBufCount(ComBuffer *Buf)
{
    return Buf->Count;
}

int eputs(char *s)
{
	// only writes to the comms port at the moment
	if (!ComOpen) return -1;
	while (*s) WriteCom(1,s++);
	return 0;
}

void eputc(char c)
{
	WriteCom(1,&c);
}

char egetc(void)
{
	return GetBuf(&ComRXBuffer);
}

int egets(char *s, int Max)
{
	// read from the comms port until end of string
	// or newline is encountered.  Buffer is terminated with null
	// returns number of characters read on success
	// returns 0 or -1 if error occurs
	// Warning: This is a blocking call.
	int Len;
	char c;
	if (!ComOpen) return -1;
	Len=0; // Until initialization code is put in init.s, we have to do this
	c = 0;
	while ( (Len < Max-1) && (c != NEWLINE) )
	{   
		while (!GetBufCount(&ComRXBuffer)); // wait for a character
		c = GetBuf(&ComRXBuffer);
		s[Len++] = c;
	}
	if (Len>0)
	{
		s[Len]=0;
	}	
	return Len;
}

char egetc_echo(void)
{
	char c;
	c=egetc();
	eputc(c);
	return c;
}

int egets_echo(char *s, int Max)
{
	// read from the comms port until end of string
	// or newline is encountered.  Buffer is terminated with null
	// returns number of characters read on success
	// returns 0 or -1 if error occurs
	// Warning: This is a blocking call.
	int Len;
	char c;
	if (!ComOpen) return -1;
	Len=0; // Until initialization code is put in init.s, we have to do this
	c = 0;
	while ( (Len < Max-1) && (c != NEWLINE) )
	{   
		while (!GetBufCount(&ComRXBuffer)); // wait for a character
		c = GetBuf(&ComRXBuffer);
		eputc(c);
		s[Len++] = c;
	}
	if (Len>0)
	{
		s[Len]=0;
	}	
	return Len;
}