#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"
#include "adc.h"
#include "macros.h"

// LQFP32 pinout
//                 ----------
//           VDD -|1       32|- VSS
//          PC14 -|2       31|- BOOT0
//          PC15 -|3       30|- PB7 (OUT 5)
//          NRST -|4       29|- PB6 (OUT 4)
//          VDDA -|5       28|- PB5 (OUT 3)
// LCD_RS    PA0 -|6       27|- PB4 (OUT 2)
// LCD_E     PA1 -|7       26|- PB3 (OUT 1)
// LCD_D4    PA2 -|8       25|- PA15
// LCD_D5    PA3 -|9       24|- PA14 (push button)
// LCD_D6    PA4 -|10      23|- PA13
// LCD_D7    PA5 -|11      22|- PA12 (pwm2)
//           PA6 -|12      21|- PA11 (pwm1)
//           PA7 -|13      20|- PA10 (Reserved for RXD)
// (ADC_IN8) PB0 -|14      19|- PA9  (Reserved for TXD)
// (ADC_IN9) PB1 -|15      18|- PA8  (Measure the period at this pin)
//           VSS -|16      17|- VDD
//                 ----------

void Hardware_Init(void)
{
	RCC->IOPENR  |= (BIT1|BIT0);         // peripheral clock enable for ports A and B

	// Configure the pin used for analog input: PB0 and PB1 (pins 14 and 15)
	GPIOB->MODER |= (BIT0|BIT1);  // Select analog mode for PB0 (pin 14 of LQFP32 package)
	GPIOB->MODER |= (BIT2|BIT3);  // Select analog mode for PB1 (pin 15 of LQFP32 package)

	initADC();

    // Make pins PA0 to PA5 outputs (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
    GPIOA->MODER = (GPIOA->MODER & ~(BIT0|BIT1)) | BIT0; // PA0
	GPIOA->OTYPER &= ~BIT0; // Push-pull
    
    GPIOA->MODER = (GPIOA->MODER & ~(BIT2|BIT3)) | BIT2; // PA1
	GPIOA->OTYPER &= ~BIT1; // Push-pull
    
    GPIOA->MODER = (GPIOA->MODER & ~(BIT4|BIT5)) | BIT4; // PA2
	GPIOA->OTYPER &= ~BIT2; // Push-pull
    
    GPIOA->MODER = (GPIOA->MODER & ~(BIT6|BIT7)) | BIT6; // PA3
	GPIOA->OTYPER &= ~BIT3; // Push-pull
    
    GPIOA->MODER = (GPIOA->MODER & ~(BIT8|BIT9)) | BIT8; // PA4
	GPIOA->OTYPER &= ~BIT4; // Push-pull
    
    GPIOA->MODER = (GPIOA->MODER & ~(BIT10|BIT11)) | BIT10; // PA5
	GPIOA->OTYPER &= ~BIT5; // Push-pull
	
	// Configure the pin used to measure period
	GPIOA->MODER &= ~(BIT16 | BIT17); // Make pin PA8 input
	// Activate pull up for pin PA8:
	GPIOA->PUPDR |= BIT16; 
	GPIOA->PUPDR &= ~(BIT17);
	
	// Configure the pin connected to the pushbutton as input
	GPIOA->MODER &= ~(BIT28 | BIT29); // Make pin PA14 input
	// Activate pull up for pin PA8:
	GPIOA->PUPDR |= BIT28; 
	GPIOA->PUPDR &= ~(BIT29);
	
	// Configure some pins as outputs:
	// Make pins PB3 to PB7 outputs (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
    GPIOB->MODER = (GPIOB->MODER & ~(BIT6|BIT7)) | BIT6;    // PB3
	GPIOB->OTYPER &= ~BIT3; // Push-pull
    GPIOB->MODER = (GPIOB->MODER & ~(BIT8|BIT9)) | BIT8;    // PB4
	GPIOB->OTYPER &= ~BIT4; // Push-pull
    GPIOB->MODER = (GPIOB->MODER & ~(BIT10|BIT11)) | BIT10; // PB5
	GPIOB->OTYPER &= ~BIT5; // Push-pull
    GPIOB->MODER = (GPIOB->MODER & ~(BIT12|BIT13)) | BIT12; // PB6
	GPIOB->OTYPER &= ~BIT6; // Push-pull
    GPIOB->MODER = (GPIOB->MODER & ~(BIT14|BIT15)) | BIT14;  // PB7
	GPIOB->OTYPER &= ~BIT7; // Push-pull
	
	// Set up servo PWM output pins
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

// A define to easily read PA14 (PA14 must be configured as input first)
#define PA14 (GPIOA->IDR & BIT14)

int main(void)
{
    int j, v;
    unsigned char mode=0;
    unsigned char pmode=0;
	long int count, f;
	unsigned char LED_toggle=0; // Used to test the outputs

	Hardware_Init(); // configure pins
    LCD_4BIT(); // init lcd
	
	waitms(500); // Give putty a chance to start before we send characters with printf()
	eputs("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
	eputs("\r\nSTM32L051 multi I/O example.\r\n");
	eputs("Measures the voltage from ADC channels 8 and 9 (pins 14 and 15 of LQFP32 package)\r\n");
	eputs("Measures period on PA8 (pin 18 of LQFP32 package)\r\n");
	eputs("Toggles PB3, PB4, PB5, PB6, PB7 (pins 26, 27, 28, 29, 30 of LQFP32 package)\r\n");
	eputs("Generates servo PWMs on PA11, PA12 (pins 21, 22 of LQFP32 package)\r\n");
	eputs("Reads the push-button on pin PA14 (pin 24 of LQFP32 package)\r\n\r\n");

    LED_toggle=0;
	PB3_0;
	PB4_0;
	PB5_0;
	PB6_0;
	PB7_0;
					
	while (1)
	{
        // happens on change of mode
        if (pmode != mode)
        {
            pmode = mode;
            if (mode == 0)
            {
                LCDPrint(1, 1, "Automatic");
            }
            else
            {
                LCDPrint(1, 1, "Manual");
            }
        }

        /*  If (d1>d) move motor 1 back.
            If (d2>d) move motor 2 back.
            If (d1<d) move motor 1 forward.
            If (d2<d) move motor 2 forward.
            d is preset after reset, but it can be 
            changed by receiving a command from the 
            remote if you wish. */

        

        j=readADC(ADC_CHSELR_CHSEL8);
		v=(j*33000)/0xfff;
		eputs("ADC[8]=0x");
		PrintNumber(j, 16, 4);
		eputs(", ");
		PrintNumber(v/10000, 10, 1);
		eputc('.');
		PrintNumber(v%10000, 10, 4);
		eputs("V ");;

		j=readADC(ADC_CHSELR_CHSEL9);
		v=(j*33000)/0xfff;
		eputs("ADC[9]=0x");
		PrintNumber(j, 16, 4);
		eputs(", ");
		PrintNumber(v/10000, 10, 1);
		eputc('.');
		PrintNumber(v%10000, 10, 4);
		eputs("V ");
		
		eputs("PA14=");
		if(PA14)
		{
			eputs("1 ");
		}
		else
		{
			eputs("0 ");
		}

		// Not very good for high frequencies because of all the interrupts in the background
		// but decent for low frequencies around 10kHz.
		count=GetPeriod(60);
		if(count>0)
		{
			f=(F_CPU*60)/count;
			eputs("f=");
			PrintNumber(f, 10, 7);
			eputs("Hz, count=");
			PrintNumber(count, 10, 6);
			eputs("          \r");
		}
		else
		{
			eputs("NO SIGNAL                     \r");
		}

		// Now turn on one of outputs per cycle to check
		switch (LED_toggle++)
		{
			case 0:
				PB3_1;
				break;
			case 1:
				PB4_1;
				break;
			case 2:
				PB5_1;
				break;
			case 3:
				PB6_1;
				break;
			case 4:
				PB7_1;
				break;
			default:
			    LED_toggle=0;
				PB3_0;
				PB4_0;
				PB5_0;
				PB6_0;
				PB7_0;
				break;
		}
		
		// Change the servo PWM signals
		if (ISR_pwm1<200)
		{
			ISR_pwm1++;
		}
		else
		{
			ISR_pwm1=100;	
		}

		if (ISR_pwm2>100)
		{
			ISR_pwm2--;
		}
		else
		{
			ISR_pwm2=200;	
		}
		
		waitms(200);	
	}
}
