//  freq_gen.c: Uses timer 2 interrupt to generate a square wave at pins
//  P2.0 and P2.1.  The program allows the user to enter a frequency.
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//  ~C51~

//COMMAND LAYOUT
//Continious 1 do nothing
//Initial Manual
//Switch to Tracking (Stuck in Tracking till Switch Button) - Read 0
//Forward - Read 00
//Backward - Read 000
//Right - Read 0000
//Left - Read 00000


#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"

unsigned char overflow_count;

void Timer2_ISR (void) interrupt INTERRUPT_TIMER2
{
	TF2H = 0; // Clear Timer2 interrupt flag
	OUT0=!OUT0;
	OUT1=!OUT0;
}

// Uses Timer3 to delay <us> micro-seconds. 
void Timer3us(unsigned char us)
{
	unsigned char i;               // usec counter
	
	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON0:
	CKCON0|=0b_0100_0000;
	
	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
	TMR3CN0 = 0x04;                 // Sart Timer3 and clear overflow flag
	for (i = 0; i < us; i++)       // Count <us> overflows
	{
		while (!(TMR3CN0 & 0x80));  // Wait for overflow
		TMR3CN0 &= ~(0x80);         // Clear overflow indicator
		if (TF0)
		{
		   TF0=0;
		   overflow_count++;
		}
	}
	TMR3CN0 = 0 ;                   // Stop Timer3 and clear overflow flag
}

void waitms (unsigned int ms)
{
	unsigned int j;
	for(j=ms; j!=0; j--)
	{
		Timer3us(249);
		Timer3us(249);
		Timer3us(249);
		Timer3us(250);
	}
}

void main (void)
{
	unsigned long int x, f;
	int test_press = 1;
	
	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.
	printf("Variable frequency generator for the EFM8LB1.\r\n"
	       "Check pins P2.0 and P2.1 with the oscilloscope.\r\n");

	while(1)
	{
		printf("New frequency=");
		scanf("%lu \n", &f);
		//printf("\nActual frequency: %lu\n", f);
		FREQ:
		if(SWITCHEROO == 0)
		{
			waitms(100);
			LOOP_OFFA:
			if(SWITCHEROO == 0)
			{
				waitms(100);
				goto LOOP_OFFA;
			}
			goto SWITCHEROO;

		}
		if(FORWARD == 0)
		{
			waitms(100);
			LOOP_OFFB:
			if(FORWARD == 0)
			{
				waitms(100);
				goto LOOP_OFFB;
			}	
			goto MOVF;
		}
		
		if(BACKWARD == 0)
		{
			waitms(100);
			LOOP_OFFC:
			if(BACKWARD == 0)
			{
				waitms(100);
				goto LOOP_OFFC;
			}	
			goto MOVB;
		}
		
		if(RIGHT == 0)
		{
			waitms(100);
			LOOP_OFFD:
			if(RIGHT == 0)
			{
				waitms(100);
				goto LOOP_OFFD;
			}	
			goto MOVR;
		}
		
		if(LEFT == 0)
		{
			waitms(100);
			LOOP_OFFE:
			if(LEFT == 0)
			{
				waitms(100);
				goto LOOP_OFFE;
			}	
			goto MOVL;
		}
	
			
		x=(SYSCLK/(2L*f));
		if(x>0xffff)
		{
			printf("Sorry %lu Hz is out of range.\n", f);
		}
		else
		{
			TR2=0; // Stop timer 2
			TMR2RL=0x10000L-x; // Change reload value for new frequency
			TR2=1; // Start timer 2
			f=SYSCLK/(2L*(0x10000L-TMR2RL));
		}
		goto FREQ;
		
		SWITCHEROO:
		printf("SWTICH \n");
		TR2=0;
		waitms(1000);
		goto FREQ;
		
		MOVF:
		printf("MOVE FORWARD \n");
		printf("MOVE FORWARD \n");
		TR2=0;
		waitms(1000);
		waitms(1000);
		goto FREQ;
		
		MOVB:
		printf("MOVE BACKWARD \n");
		printf("MOVE BACKWARD \n");
		printf("MOVE BACKWARD \n");
		TR2=0;
		waitms(1000);
		waitms(1000);
		waitms(1000);
		goto FREQ;
		
		MOVR:
		printf("MOVE RIGHT \n");
		printf("MOVE RIGHT \n");
		printf("MOVE RIGHT \n");
		printf("MOVE RIGHT \n");
		TR2=0;
		waitms(1000);
		waitms(1000);
		waitms(1000);
		waitms(1000);
		goto FREQ;
		
		MOVL:
		printf("MOVE LEFT \n");
		printf("MOVE LEFT \n");
		printf("MOVE LEFT \n");
		printf("MOVE LEFT \n");
		printf("MOVE LEFT \n");
		TR2=0;
		waitms(1000);
		waitms(1000);
		waitms(1000);
		waitms(1000);
		waitms(1000);
		goto FREQ;
	}
}