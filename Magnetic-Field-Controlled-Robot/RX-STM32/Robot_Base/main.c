#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"
#include "adc.h"
#include "macros.h"
#include "../LCD/lcd.h"

// LQFP32 pinout
//                 ----------
//           VDD -|1       32|- VSS
//          PC14 -|2       31|- BOOT0
//          PC15 -|3       30|- PB7 (OUT 5)
//          NRST -|4       29|- PB6 (OUT 4) (LF)
//          VDDA -|5       28|- PB5 (OUT 3) (LB)
// LCD_RS    PA0 -|6       27|- PB4 (OUT 2) (RF)
// LCD_E     PA1 -|7       26|- PB3 (OUT 1) (RB)
// LCD_D4    PA2 -|8       25|- PA15
// LCD_D5    PA3 -|9       24|- PA14 (push button)
// LCD_D6    PA4 -|10      23|- PA13
// LCD_D7    PA5 -|11      22|- PA12 (pwm2)
//   		 PA6 -|12      21|- PA11 (pwm1)
//           PA7 -|13      20|- PA10 (Reserved for RXD)
// (ADC_IN8) PB0 -|14      19|- PA9  (Reserved for TXD)
// (ADC_IN9) PB1 -|15      18|- PA8  (Measure the period at this pin)
//           VSS -|16      17|- VDD
//                 ----------

//  ----------------------------------------- GLOBAL VARS ------------------------------------------------------------
unsigned char mode=0;

// Interrupt service routines are the same as normal
// subroutines (or C funtions) in Cortex-M microcontrollers.
// The following should happen at a rate of 1kHz.
// The following function is associated with the TIM21 interrupt 
// via the interrupt vector table defined in startup.s
void TIM21_Handler(void) 
{
	TIM21->SR &= ~BIT0; // clear update interrupt flag
	Count++;
	if (Count > 10) // happens every 10ms
	{ 
		Count = 0;
		if (readADC(ADC_CHSELR_CHSEL8) < 100) {
            OffCycles++;
        } else {
            switch(OffCycles)
            {
                case 0:
                    break;
                case 1:
                    mode = 0;
                    LCDprint("Automatic", 1, 1);
                    break;
                case 2:
                    mode = 1;
                    LCDprint("Manual", 1, 1);
                    break;
                case 3:

                    break;
                case 4:
                    break;
                default:
                    mode = 0;
                    break;
            }
            OffCycles = 0;
        }
	}   
}

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

	// Set up timer 2
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = F_CPU/DEF_F-1;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	// Set up timer 21
	RCC->APB2ENR |= BIT2;  // turn on clock for timer21 (UM: page 188)
	TIM21->ARR = SYSCLK/TICK_FREQ;
	NVIC->ISER[0] |= BIT20; // enable timer 21 interrupts in the NVIC
	TIM21->CR1 |= BIT4;      // Downcounting    
	TIM21->CR1 |= BIT0;      // enable counting    
	TIM21->DIER |= BIT0;     // enable update event (reload event) interrupt  

    LCD_4BIT(); // init lcd

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

#define ADC50CM 1000

int main(void)
{
    int j, v;
	long int count, f;
    int L, R;

	Hardware_Init(); // configure pins, adc, lcd
	
	waitms(500); // Give putty a chance to start before we send characters with printf()
	eputs("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
	eputs("\r\nSTM32L051 multi I/O example.\r\n");
	eputs("Measures the voltage from ADC channels 8 and 9 (pins 14 and 15 of LQFP32 package)\r\n");
	eputs("Measures period on PA8 (pin 18 of LQFP32 package)\r\n");
	eputs("Toggles PB3, PB4, PB5, PB6, PB7 (pins 26, 27, 28, 29, 30 of LQFP32 package)\r\n");
	eputs("Generates servo PWMs on PA11, PA12 (pins 21, 22 of LQFP32 package)\r\n");
	eputs("Reads the push-button on pin PA14 (pin 24 of LQFP32 package)\r\n\r\n");

	PB3_1;
	PB4_1;
	PB5_1;
	PB6_1;
	PB7_0; // unused for now
    
    // print initial
    LCDprint("Automatic", 1, 1);
					
	while (1)
	{
        /*
        char buf[32];
        printf("L Reading: ");
    	fflush(stdout);
    	egets_echo(buf, 31); // wait here until data is received
        L = atoi(buf);

        printf("R Reading: ");
    	fflush(stdout);
    	egets_echo(buf, 31); // wait here until data is received
        R = atoi(buf);
*/
        if (mode == 0) {
            if (readADC(ADC_CHSELR_CHSEL8) > 100) { // read only if its on
                L = readADC(ADC_CHSELR_CHSEL8);
                R = readADC(ADC_CHSELR_CHSEL9);
            }

            if (L > ADC50CM) { // move L back
                PB6_1;
                PB5_0;
            } else { // move L forward
                PB6_0;
                PB5_1;
            }

            if (R > ADC50CM) { // move R back
                PB4_1;
                PB3_0;
            } else { // move R forward
                PB4_0;
                PB3_1;
            }
        } else {
                PB6_0;
                PB5_0;
                PB4_0;
                PB3_0;
        }

        /*
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
		*/
    
        /*
		eputs("PA14=");
		if(PA14)
		{
			eputs("1 ");
		}
		else
		{
			eputs("0 ");
		}
        */

        /*
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
        */
		
		waitms(200);	
	}
}
