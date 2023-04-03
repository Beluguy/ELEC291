#include <stdio.h>
#include "../Common/Include/stm32l051xx.h"
#include "serial.h"

#define F_CPU 32000000L

#define CLR_CS (GPIOA->ODR &= ~BIT3) // CSn=0 enable 25Q32
#define SET_CS (GPIOA->ODR |= BIT3) // CSn=1 disable 25Q32

// Flash memory commands
#define WRITE_ENABLE     0x06  // Address:0 Dummy:0 Num:0 fMax: 25MHz
#define WRITE_DISABLE    0x04  // Address:0 Dummy:0 Num:0 fMax: 25MHz
#define READ_STATUS      0x05  // Address:0 Dummy:0 Num:1 to infinite fMax: 32MHz
#define READ_BYTES       0x03  // Address:3 Dummy:0 Num:1 to infinite fMax: 20MHz
#define READ_SILICON_ID  0xab  // Address:0 Dummy:3 Num:1 to infinite fMax: 32MHz
#define FAST_READ        0x0b  // Address:3 Dummy:1 Num:1 to infinite fMax: 40MHz
#define WRITE_STATUS     0x01  // Address:0 Dummy:0 Num:1 fMax: 25MHz
#define WRITE_BYTES      0x02  // Address:3 Dummy:0 Num:1 to 256 fMax: 25MHz
#define ERASE_ALL        0xc7  // Address:0 Dummy:0 Num:0 fMax: 25MHz
#define ERASE_BLOCK      0xd8  // Address:3 Dummy:0 Num:0 fMax: 25MHz
#define READ_DEVICE_ID   0x9f  // Address:0 Dummy:2 Num:1 to infinite fMax: 25MHz

volatile unsigned long int playcnt=0;
volatile unsigned char play_flag=0;

void wait_1ms(void)
{
	// For SysTick info check the STM32L0xxx Cortex-M0 programming manual page 85.
	SysTick->LOAD = (F_CPU/1000L) - 1;  // set reload register, counter rolls over from zero, hence -1
	SysTick->VAL = 0; // load the SysTick counter
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick IRQ and SysTick Timer */
	while((SysTick->CTRL & BIT16)==0); // Bit 16 is the COUNTFLAG.  True when counter rolls over from zero.
	SysTick->CTRL = 0x00; // Disable Systick counter
}

void delayMs(int len)
{
	while(len--) wait_1ms();
}

// Pins used for SPI: 

// Pin 9  is CSn  ---> pin 1 of 25Q32
// Pin 11 is SCK  ---> pin 6 of 25Q32
// Pin 12 is MISO ---> pin 2 of 25Q32
// PIN 13 is MOSI ---> Pin 5 of 25Q32
// 3.3V pins 3, 7, 8 of 25Q32
// GND pin 4 of 25Q32

// LQFP32 pinout
//              ----------
//        VDD -|1       32|- VSS
//       PC14 -|2       31|- BOOT0
//       PC15 -|3       30|- PB7
//       NRST -|4       29|- PB6
//       VDDA -|5       28|- PB5
//        PA0 -|6       27|- PB4
//        PA1 -|7       26|- PB3
//        PA2 -|8       25|- PA15
//        PA3 -|9       24|- PA14
//        PA4 -|10      23|- PA13
//        PA5 -|11      22|- PA12
//        PA6 -|12      21|- PA11
//        PA7 -|13      20|- PA10 (Reserved for RXD)
//        PB0 -|14      19|- PA9  (Reserved for TXD)
//        PB1 -|15      18|- PA8
//        VSS -|16      17|- VDD
//              ----------


// https://community.st.com/s/question/0D50X00009XkXwy/stm32f0-spi-read-and-write
static void Config_SPI(void)
{
	RCC->IOPENR |= BIT0;  // Enable GPIOA clock (page 175 of RM0451 Reference manual)
	RCC->APB2ENR |= BIT12; // peripheral clock enable for SPI1 (page 175 of RM0451 Reference manual)
	
	GPIOA->OSPEEDR=0xffffffff; // All pins of port A configured for very high speed! Page 201 of RM0451
	
	// Configure PA3 for CSn, pin 9 in LQFP32 package
    GPIOA->MODER = (GPIOA->MODER & ~(BIT6|BIT7)) | BIT6; // Make pin PA3 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	GPIOA->OTYPER   &= ~BIT3; // Push-pull
	GPIOA->ODR |= BIT3; // CSn=1
	
	//Configure PA5 for SPI1_SCK, pin 11 in LQFP32 package
    GPIOA->MODER = (GPIOA->MODER & ~(BIT10|BIT11)) | BIT11; // Make pin PA5 AF-Mode (page 200 of RM0451, two bits used to configure: bit0=0, bit1=1)
	GPIOA->AFR[0]  |= 0; // AF0 selected (page 204 of RM0451 Reference manual)
	
	//Configure PA6 for SPI1_MISO, pin 12 in LQFP32 package
    GPIOA->MODER = (GPIOA->MODER & ~(BIT12|BIT13)) | BIT13; // Make pin PA6 AF-Mode (page 200 of RM0451, two bits used to configure: bit0=0, bit1=1)
	GPIOA->AFR[0]  |= 0; // AF0 selected (page 204 of RM0451 Reference manual)
	
	//Configure PA7 for SPI1_MOSI, pin 13 in LQFP32 package
    GPIOA->MODER = (GPIOA->MODER & ~(BIT14|BIT15)) | BIT15; // Make pin PA7 AF-Mode (page 200 of RM0451, two bits used to configure: bit0=0, bit1=1)
	GPIOA->AFR[0]  |= 0; // AF0 selected (page 204 of RM0451 Reference manual)
	
	SPI1->CR1 = 0x00000000; // Reset SPI1 CR1 register.  Page 707 of RM0451 Reference manual
	SPI1->CR1 = (( 0ul << 0) | // CPHA=0
				( 0ul << 1) | // CPOL=0
				( 1ul << 2) | // MSTR=1
				( 2ul << 3) | // BR (fPCLK/8) ~= 5Mbit/sec ]
				( 0ul << 7) | // MSBFIRST
				( 1ul << 8) | // SSI must be 1 when SSM=1 or a frame error occurs
				( 1ul << 9)); // SSM
	SPI1->CR2 = ( BIT10 | BIT9 | BIT8 ); // page 709 of RM0451 Reference manual
	SPI1->CR1 |= BIT6; // Enable SPI1
}

uint8_t SPIWrite (uint8_t wr)
{
	uint8_t data=0;
	
	while ((SPI1->SR & BIT1) == 0); // SPI status register (SPIx_SR) is in page 806
	*((uint8_t *)&SPI1->DR) = wr; // "SPI1_DR = wr;" send 16 bits instead of 8!
	while (SPI1->SR & BIT7); // Check Busy flag (Page 806)
	//while ((SPI1_SR & BIT0) == 0); // 0: Rx buffer empty (hangs here)
	data = *((uint8_t *)&SPI1->DR); // "data = SPI1_DR;" waits for 16-bits
	
	return data;
}

// Interrupt service routines are the same as normal
// subroutines (or C funtions) in Cortex-M microcontrollers.
// The following should happen at a rate of 1kHz.
// The following function is associated with the TIM21 interrupt 
// via the interrupt vector table defined in startup.c
void TIM21_Handler(void) 
{
	TIM21->SR &= ~BIT0; // clear update interrupt flag
	GPIOA->ODR ^= BIT0; // Toggle PA0 (pin 6) to check for the right frequency (22050 Hz)
	
	if(play_flag!=0)
	{  
		if(playcnt==0)
		{
			SET_CS; // Done playing: Disable 25Q32 SPI flash memory
			TIM2->CCR1=0x80;
			play_flag=0;
		}
		else
		{
			// Output value to PWM (used as DAC)
			TIM2->CCR1=SPIWrite(0x00);
			playcnt--;
		}
	}
}

void Timer21_Init(void)
{
	RCC->IOPENR |= BIT0;  // Enable GPIOA clock (page 175 of RM0451 Reference manual)
    GPIOA->MODER = (GPIOA->MODER & 0xfffffffc) | 0x00000001; // Make pin PA0 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	
	// Set up timer
	RCC->APB2ENR |= BIT2;  // turn on clock for timer21 (UM: page 181)
	TIM21->ARR = F_CPU/(22050L)-1;
	NVIC->ISER[0] |= BIT20;  // enable timer 21 interrupts in the NVIC
	TIM21->CR1 |= BIT4;      // Downcounting    
	TIM21->CR1 |= BIT0;      // enable counting    
	TIM21->DIER |= BIT0;     // enable update event (reload event) interrupt  
	__enable_irq();
}

// Timer 2, more specifically channel 1 of timer 2, is used to produce an 8-bit PWM signal
// used as DAC to produce the sound.
void Timer2_Init(void)
{
	// Configure PA15 for altenate function (TIM2_CH1, pin 25 in LQFP32 package)
	GPIOA->OSPEEDR  |= BIT30; // MEDIUM SPEED
	GPIOA->OTYPER   &= ~BIT15; // Push-pull
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT30)) | BIT31; // AF-Mode
	GPIOA->AFR[1]   |= BIT30 | BIT28 ; // AF5 selected (check table 16 in page 43 of "en.DM00108219.pdf")
	
	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = 255;
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	// Enable PWM in channel 1 of Timer 2
	TIM2->CCMR1|=BIT6|BIT5; // PWM mode 1 ([6..4]=110)
	TIM2->CCMR1|=BIT3; // OC1PE=1
	TIM2->CCER|=BIT0; // Bit 0 CC1E: Capture/Compare 1 output enable.
	
	// Set PWM to 50%
	TIM2->CCR1=128;
	TIM2->EGR |= BIT0; // UG=1
}

void Start_Playback (unsigned long int address, unsigned long int numb)
{
    CLR_CS; // Select/enable 25Q32 SPI flash memory.
    SPIWrite(READ_BYTES);
    SPIWrite((unsigned char)((address>>16)&0xff));
    SPIWrite((unsigned char)((address>>8)&0xff));
    SPIWrite((unsigned char)(address&0xff));
    playcnt=numb;
    play_flag=1;
}

void Enable_Write (void)
{
    CLR_CS; // Enable 25Q32 SPI flash memory.
    SPIWrite(WRITE_ENABLE);
	SET_CS; // Disable 25Q32 SPI flash memory
}

void Check_WIP (void)
{
	unsigned char c;
	do
	{
    	CLR_CS; // Enable 25Q32 SPI flash memory.
	    SPIWrite(READ_STATUS);
	    c=SPIWrite(0x55);
		SET_CS; // Disable 25Q32 SPI flash memory
	} while (c&0x01);
}

static const unsigned short crc16_ccitt_table[256] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52B5U, 0x4294U, 0x72F7U, 0x62D6U,
    0x9339U, 0x8318U, 0xB37BU, 0xA35AU, 0xD3BDU, 0xC39CU, 0xF3FFU, 0xE3DEU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64E6U, 0x74C7U, 0x44A4U, 0x5485U,
    0xA56AU, 0xB54BU, 0x8528U, 0x9509U, 0xE5EEU, 0xF5CFU, 0xC5ACU, 0xD58DU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76D7U, 0x66F6U, 0x5695U, 0x46B4U,
    0xB75BU, 0xA77AU, 0x9719U, 0x8738U, 0xF7DFU, 0xE7FEU, 0xD79DU, 0xC7BCU,
    0x48C4U, 0x58E5U, 0x6886U, 0x78A7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xC9CCU, 0xD9EDU, 0xE98EU, 0xF9AFU, 0x8948U, 0x9969U, 0xA90AU, 0xB92BU,
    0x5AF5U, 0x4AD4U, 0x7AB7U, 0x6A96U, 0x1A71U, 0x0A50U, 0x3A33U, 0x2A12U,
    0xDBFDU, 0xCBDCU, 0xFBBFU, 0xEB9EU, 0x9B79U, 0x8B58U, 0xBB3BU, 0xAB1AU,
    0x6CA6U, 0x7C87U, 0x4CE4U, 0x5CC5U, 0x2C22U, 0x3C03U, 0x0C60U, 0x1C41U,
    0xEDAEU, 0xFD8FU, 0xCDECU, 0xDDCDU, 0xAD2AU, 0xBD0BU, 0x8D68U, 0x9D49U,
    0x7E97U, 0x6EB6U, 0x5ED5U, 0x4EF4U, 0x3E13U, 0x2E32U, 0x1E51U, 0x0E70U,
    0xFF9FU, 0xEFBEU, 0xDFDDU, 0xCFFCU, 0xBF1BU, 0xAF3AU, 0x9F59U, 0x8F78U,
    0x9188U, 0x81A9U, 0xB1CAU, 0xA1EBU, 0xD10CU, 0xC12DU, 0xF14EU, 0xE16FU,
    0x1080U, 0x00A1U, 0x30C2U, 0x20E3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83B9U, 0x9398U, 0xA3FBU, 0xB3DAU, 0xC33DU, 0xD31CU, 0xE37FU, 0xF35EU,
    0x02B1U, 0x1290U, 0x22F3U, 0x32D2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xB5EAU, 0xA5CBU, 0x95A8U, 0x8589U, 0xF56EU, 0xE54FU, 0xD52CU, 0xC50DU,
    0x34E2U, 0x24C3U, 0x14A0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xA7DBU, 0xB7FAU, 0x8799U, 0x97B8U, 0xE75FU, 0xF77EU, 0xC71DU, 0xD73CU,
    0x26D3U, 0x36F2U, 0x0691U, 0x16B0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xD94CU, 0xC96DU, 0xF90EU, 0xE92FU, 0x99C8U, 0x89E9U, 0xB98AU, 0xA9ABU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18C0U, 0x08E1U, 0x3882U, 0x28A3U,
    0xCB7DU, 0xDB5CU, 0xEB3FU, 0xFB1EU, 0x8BF9U, 0x9BD8U, 0xABBBU, 0xBB9AU,
    0x4A75U, 0x5A54U, 0x6A37U, 0x7A16U, 0x0AF1U, 0x1AD0U, 0x2AB3U, 0x3A92U,
    0xFD2EU, 0xED0FU, 0xDD6CU, 0xCD4DU, 0xBDAAU, 0xAD8BU, 0x9DE8U, 0x8DC9U,
    0x7C26U, 0x6C07U, 0x5C64U, 0x4C45U, 0x3CA2U, 0x2C83U, 0x1CE0U, 0x0CC1U,
    0xEF1FU, 0xFF3EU, 0xCF5DU, 0xDF7CU, 0xAF9BU, 0xBFBAU, 0x8FD9U, 0x9FF8U,
    0x6E17U, 0x7E36U, 0x4E55U, 0x5E74U, 0x2E93U, 0x3EB2U, 0x0ED1U, 0x1EF0U
};

unsigned short crc16_ccitt(unsigned char val, unsigned short crc)
{
    unsigned short tmp;

    tmp = (crc >> 8) ^ val;
    crc = ((unsigned short)(crc << 8U)) ^ crc16_ccitt_table[tmp];
    return crc;
}

// Get a 24-bit number from the serial port and store it into a unsigned long
void get_ulong(unsigned long * lptr)
{
    unsigned char * bytes;
    bytes=(unsigned char *) lptr;
	bytes[3]=0;
	bytes[2]=uart_getc();
	bytes[1]=uart_getc();
	bytes[0]=uart_getc();
}

void main(void)
{
    unsigned char c;
    unsigned int j, n;
    unsigned long start, nbytes;
    unsigned short crc;
    
	Config_SPI();
    Timer21_Init(); // This timer is used for the ISR that outputs sound
    Timer2_Init(); // Used to generate a PWM signal which filtered behaves as a DAC
    
	playcnt=0;
	play_flag=0;
	SET_CS; // Disable 25Q32 SPI flash memory
      
	while(1)
	{
		c=uart_getc();
		if(c=='#')
		{
			playcnt=0;
			play_flag=0;
			SET_CS; // Disable 25Q32 SPI flash memory

			c=uart_getc();
			switch(c)
			{
				case '0': // Identify command
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(READ_DEVICE_ID);
				    c=SPIWrite((unsigned char)(0x00));
				    uart_putc(c);
				    c=SPIWrite((unsigned char)(0x00));
				    uart_putc(c);
				    c=SPIWrite((unsigned char)(0x00));
				    uart_putc(c);
				    SET_CS; // Disable 25Q32 SPI flash memory
				break;

				case '1': // Erase whole flash (takes a long time)
					Enable_Write();
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(ERASE_ALL);
				    SET_CS; // Disable 25Q32 SPI flash memory
				    Check_WIP();
				    uart_putc(0x01);
				break;
				
				case '2': // Load flash page (256 bytes or less)
					Enable_Write();
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(WRITE_BYTES);
				    c=uart_getc(); // Address bits 16 to 23
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 8 to 15
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 0 to 7
				    SPIWrite(c);
				    n=uart_getc(); // Number of bytes to write
				    if(n==0) n=256;
				    for(j=0; j<n; j++)
				    {
				    	c=uart_getc();
				    	SPIWrite(c);
				    }
				    SET_CS; // Disable 25Q32 SPI flash memory
				    Check_WIP();
				    uart_putc(0x01);
				break;
				
				case '3': // Read flash bytes (256 bytes or less)
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(READ_BYTES);
				    c=uart_getc(); // Address bits 16 to 23
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 8 to 15
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 0 to 7
				    SPIWrite(c);
				    n=uart_getc(); // Number of bytes to write
				    if(n==0) n=256;
				    for(j=0; j<n; j++)
				    {
				    	c=SPIWrite(0x55);
				    	uart_putc(c);
				    }
				    SET_CS; // Disable 25Q32 SPI flash memory
				break;
				
				case '4': // Playback a portion of the stored wav file
					get_ulong(&start); // Get the start position
					get_ulong(&nbytes); // Get the number of bytes to playback
					Start_Playback(start, nbytes);
				break;
				
				// WARNING: CRC calculation here is way slower than in the EFM8 or SoC-8052.
				// Modify computer_sender.c so it doesn't time out.
				// Change line:
				//               maxwait=length/300000.0;
				// to:
				//               maxwait=length/30000.0;
				case '5': ; // Calculate and send CRC-16 of ISP flash memory from zero to the 24-bit passed value.
					get_ulong(&nbytes); // Get the total number of bytes used to calculate the crc
				
					crc=0;
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(READ_BYTES);
				    SPIWrite(0x00); // Address bits 16 to 23
				    SPIWrite(0x00); // Address bits 8 to 1
				    SPIWrite(0x00); // Address bits 0 to 7
				    
					for(start=0; start<nbytes; start++)
					{
						c=SPIWrite(0x00);
						crc=crc16_ccitt(c, crc); // Calculate CRC here
					}
				    SET_CS; // Disable 25Q32 SPI flash memory

					uart_putc(crc/0x100); // Send high byte of CRC
					uart_putc(crc%0x100); // Send low byte of CRC
				break;

				case '6': // Fill flash page (256 bytes or less).
					Enable_Write();
				    CLR_CS; // Enable 25Q32 SPI flash memory.
				    SPIWrite(WRITE_BYTES);
				    c=uart_getc(); // Address bits 16 to 23
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 8 to 15
				    SPIWrite(c);
				    c=uart_getc(); // Address bits 0 to 7
				    SPIWrite(c);
				    c=uart_getc(); // byte to copy to page
				    for(j=0; j<256; j++)
				    {
				    	SPIWrite(c);
				    }
				    SET_CS; // Disable 25Q32 SPI flash memory
				    Check_WIP();
				    uart_putc(0x01);
				break;
			}
		}
    }  
}
