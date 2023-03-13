//This program uses an stm32 to measure the capacitance of a capacitor between 0.1uF and 1uF

// LQFP32 pinout
//              ----------
//        VDD -|1       32|- VSS
//       PC14 -|2       31|- BOOT0
//       PC15 -|3       30|- PB7
//       NRST -|4       29|- PB6
//       VDDA -|5       28|- PB5
// LCD_RS PA0 -|6       27|- PB4
// LCD_E  PA1 -|7       26|- PB3
// LCD_D4 PA2 -|8       25|- PA15
// LCD_D5 PA3 -|9       24|- PA14
// LCD_D6 PA4 -|10      23|- PA13
// LCD_D7 PA5 -|11      22|- PA12
//        PA6 -|12      21|- PA11
//        PA7 -|13      20|- PA10 (Reserved for RXD)
//        PB0 -|14      19|- PA9  (Reserved for TXD)
//        PB1 -|15      18|- PA8
//        VSS -|16      17|- VDD
//              ----------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcd.h"
#include "../Common/Include/serial.h"
#include "../Common/Include/stm32l051xx.h"

#define F_CPU 32000000L
#define RESISTANCE 1666.7
#define CHARS_PER_LINE 16

void Configure_Pins (void)
{
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
	
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
}


void main(void)
{  
	char buff[17];
	int i;

	Configure_Pins();
	LCD_4BIT();
	
	//WARNING: notice that printf() of floating point numbers is not enabled in the makefile!
	waitms(500);
	printf("4-bit mode LCD Test using the STM32L051.\r\n");
	
   	// Display something in the LCD
	LCDprint("LCD 4-bit test:", 1, 1);
	LCDprint("Hello, World!", 2, 1);
	while(1)
	{
		printf("Type what you want to display in line 2 (16 char max): ");
		fflush(stdout); // GCC peculiarities: need to flush stdout to get string out without a '\n'
		egets_echo(buff, sizeof(buff));
		printf("\r\n");
		for(i=0; i<sizeof(buff); i++)
		{
			if(buff[i]=='\n') buff[i]=0;
			if(buff[i]=='\r') buff[i]=0;
		}
		LCDprint(buff, 2, 1);
	}
}










/*
unsigned char overflow_count;


void main(void)
{
    float period;
    double capacitance = 0.0;
    char buff[17];
    double cap_old = 0.0;
    int units = 0;
    float conversion_factor = 1000000000.0;
    P3_7 = 1;
    P2_0 = 1;

    TIMER0_Init(); //
    LCD_4BIT();    // init lcd

    LCDprint("C measured [nF]:", 1, 1);

    waitms(500);       // Give PuTTY a chance to start.
    printf("\x1b[2J"); // Clear screen using ANSI escape sequence.

    printf("EFM8 Period measurement at pin P0.1 using Timer 0.\n"
           "File: %s\n"
           "Compiled: %s, %s\n\n",
           __FILE__, __DATE__, __TIME__);

    while (1)
    {
        // Reset the counter
        TL0 = 0;
        TH0 = 0;
        TF0 = 0;
        overflow_count = 0;

        while (P3_7 != 0)
        {
            if (P2_0 == 0)
            {
                units = !units;
                if (units == 0)
                {
                    LCDprint("C measured [nF]:", 1, 1);
                    conversion_factor = 1000000000.0;
                    cap_old = cap_old * 1000.0;
                    capacitance = capacitance * 1000.0;
                    sprintf(buff, "%.1f %.1f", capacitance, cap_old);
                    LCDprint(buff, 2, 1);
                }
                else
                {
                    LCDprint("C measured [uF]:", 1, 1);
                    conversion_factor = 1000000.0;
                    cap_old = cap_old / 1000.0;
                    capacitance = capacitance / 1000.0;
                    sprintf(buff, "%.3f %.3f", capacitance, cap_old);
                    LCDprint(buff, 2, 1);
                }
                waitms(500);
            }
            waitms(50);
        }           // wait for boot to be pressed for next read
        waitms(50); // make sure switch doesn't bounce

        while (P0_1 != 0); // Wait for the signal to be zero
        while (P0_1 != 1);             // Wait for the signal to be one
        TR0 = 1;          // Start the timer
        while (P0_1 != 0) // Wait for the signal to be zero
        {
            if (TF0 == 1) // Did the 16-bit timer overflow?
            {
                TF0 = 0;
                overflow_count++;
            }
        }
        while (P0_1 != 1) // Wait for the signal to be one
        {
            if (TF0 == 1) // Did the 16-bit timer overflow?
            {
                TF0 = 0;
                overflow_count++;
            }
        }
        TR0 = 0; // Stop timer 0, the 24-bit number [overflow_count-TH0-TL0] has the period!
        period = (overflow_count * 65536.0 + TH0 * 256.0 + TL0) * (12.0 / SYSCLK);
        // Send the period to the serial port
        printf("T=%f ms    \n", period * 1000.0);
        cap_old = capacitance;
        capacitance = conversion_factor * period / (0.693 * RESISTANCE * 3.0);
        printf("C=%f nF    \n", capacitance);
        if (units == 0)
        {
            sprintf(buff, "%.1f %.1f", capacitance, cap_old);
        }
        else
        {
            sprintf(buff, "%.3f %.3f", capacitance, cap_old);
        }
        LCDprint(buff, 2, 1);
        waitms(500);
    }
}
*/