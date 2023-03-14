//This program uses an stm32 to measure the capacitance of a capacitor between 0.1uF and 1uF
//Bonus: Contrast, battery, recharge module, unit changing, update button, memory, gui with python 
/* LQFP32 pinout
                                          ----------
                                    VDD -|1       32|- VSS
                                   PC14 -|2       31|- BOOT0
                                   PC15 -|3       30|- PB7
                                   NRST -|4       29|- PB6
                                   VDDA -|5       28|- PB5
                             LCD_RS PA0 -|6       27|- PB4
                             LCD_E  PA1 -|7       26|- PB3
                             LCD_D4 PA2 -|8       25|- PA15
                             LCD_D5 PA3 -|9       24|- PA14 
                             LCD_D6 PA4 -|10      23|- PA13
                             LCD_D7 PA5 -|11      22|- PA12
                                    PA6 -|12      21|- PA11 (update pushbutton)
   (Measure the period at this pin) PA7 -|13      20|- PA10 (Reserved for RXD)
                                    PB0 -|14      19|- PA9  (Reserved for TXD)
                                    PB1 -|15      18|- PA8  (unit changing pushbutton)
                                    VSS -|16      17|- VDD
                                          ----------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "func.h"
#include "../Common/Include/serial.h"
#include "../Common/Include/stm32l051xx.h"

#define RESISTANCE 1666.7

void Configure_Pins (void)
{
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
    
	//------------------------for LCD-------------------------
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
    //---------------------------------------------------------

    //---------------------for pushbuttons---------------------
     GPIOA->MODER &= ~(BIT16 | BIT17); // Make pin PA8 unit pushbutton input
	// Activate pull up for pin PA31:
	GPIOA->PUPDR |= BIT16; 
	GPIOA->PUPDR &= ~(BIT17); 
    
    GPIOA->MODER &= ~(BIT22 | BIT23); // Make pin PA11 memory pushbutton input
	// Activate pull up for pin PA11:
	GPIOA->PUPDR |= BIT22; 
	GPIOA->PUPDR &= ~(BIT23); 
    //---------------------------------------------------------

    //-----------------------for 555timer----------------------
    GPIOA->MODER &= ~(BIT14 | BIT15); // Make pin PA7 input
	// Activate pull up for pin PA8:
	GPIOA->PUPDR |= BIT14; 
	GPIOA->PUPDR &= ~(BIT15); 
    //---------------------------------------------------------
}

void main(void)
{  
    Configure_Pins();
	LCD_4BIT();

    //----------------from period--------------
    long int count;
	float T, f;
    //-----------------------------------------

    //---------------------from lab4----------------------------
    double capacitance = 0.0;
    char buff[17];
    double cap_old = 0.0;
    int units = 0;
    float conversion_factor = 1000000000.0;
    //----------------------------------------------------------
	
	//WARNING: notice that printf() of floating point numbers is not enabled in the makefile!
    waitms(500);
    //printf("STM32L051 Capacitance measurement at pin PA7.\r\n");
	LCDprint("C measured [nF]:\r\n", 1, 1);
    while(1)
	{   
		while(GPIOA->IDR&BIT11)     		// while update isn't pressed
		{	
            if(!(GPIOA->IDR&BIT8))       	//if unit is pressed
			{	
				units =! units;
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
				waitms(200);
			}
			waitms(50);
		}
		waitms(50);
		
		count=GetPeriod(100);
		T = count / (F_CPU*100.0); // Since we have the time of 100 periods, we need to divide by 100
		f = 1.0 / T;
		cap_old = capacitance;
        capacitance = conversion_factor * T / (0.693 * RESISTANCE * 3.0);

        if (units == 0)
        {
            sprintf(buff, "%.1f %.1f", capacitance, cap_old);
			printf("{\"cap\":%.2f,\"freq\":%.2f,\"unit\":\"nF\"}\r\n", capacitance, f);
        }
        else
        {
            sprintf(buff, "%.3f %.3f", capacitance, cap_old);
			printf("{\"cap\":%.2f,\"freq\":%.2f,\"unit\":\"uF\"}\r\n", capacitance, f);
        }
        LCDprint(buff, 2, 1);
        fflush(stdout); // GCC printf wants a \n in order to send something.  If \n is not present, we fflush(stdout)
		waitms(50);
	}
}
