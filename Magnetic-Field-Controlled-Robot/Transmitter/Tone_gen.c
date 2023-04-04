//  freq_gen.c: Uses timer 2 interrupt to generate a square wave at pins
//  P2.0 and P2.1.  The program allows the user to enter a frequency.
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//  ~C51~

//COMMAND LAYOUT
//Continious 1 do nothing
//Initial Manual
//Switch to Tracking (Stuck in Tracking till Switch Button) - Read 00
//Forward - Read 000
//Backward - Read 0000
//Right - Read 00000
//Left - Read 000000


#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include "lcd.h"
#include <ctype.h>
/* tetris.c
****************************************************************************
	Tetris game for the 8052 microcontroller!
	
	Originally from:
	http://my.execpc.com/~geezer/software/tetris.c
	Christopher Giese <geezer[AT]execpc.com>
	Release date 8/12/98. Distribute freely. ABSOLUTELY NO WARRANTY.
	
	Ported to the 8052 microcontroller using C51 by:
	Jesus Calvino-Fraga <jesusc[AT]ece.ubc.ca>
	Release date Dec/02/2005. Distribute freely. ABSOLUTELY NO WARRANTY.
	
***************************************************************************/

unsigned char delay=40;

/* For code size and speed: */
#define printf printf_tiny

#define	KEY_QUIT	1
#define	KEY_CW		2
#define	KEY_CCW		3
#define	KEY_RIGHT	4
#define	KEY_LEFT	5
#define	KEY_UP		6
#define	KEY_DOWN	7
#define KEY_BEGIN	8

/* Dimensions of the playing area.  Whatch out here, as if you
increase the dimensions of the playing area too much, there
may not be enough memory in the microcontroller to handle it.
If you want a bigger playing area, or what you have is an 8051,
you can declare Screen[][] in xdata, but you will need external
memory to do so.*/

#define	SCN_WID	15
#define	SCN_HT	24

/* Direction vectors */
#define	DIR_UP	{ 0, -1 }
#define	DIR_DN	{ 0, +1 }
#define	DIR_LT	{ -1, 0 }
#define	DIR_RT	{ +1, 0 }
#define	DIR_UP2	{ 0, -2 }
#define	DIR_DN2	{ 0, +2 }
#define	DIR_LT2	{ -2, 0 }
#define	DIR_RT2	{ +2, 0 }

/* ANSI colors */
#define	COLOR_BLACK		0
#define	COLOR_RED		1
#define	COLOR_GREEN		2
#define	COLOR_YELLOW	3
#define	COLOR_BLUE		4
#define	COLOR_MAGENTA	5
#define	COLOR_CYAN		6
#define	COLOR_WHITE		7

/* Some ANSI escape sequences */
#define CURSOR_ON "\x1b[?25h"
#define CURSOR_OFF "\x1b[?25l"
#define CLEAR_SCREEN "\x1b[2J"
#define GOTO_YX "\x1B[%d;%dH"
#define CLR_TO_END_LINE "\x1B[K"
/* Black foreground, white background */
#define BKF_WTB "\x1B[0;30;47m"

// Dummy entry point for single step and breakpoints needed by deb51
void Timer1_ISR (void) interrupt INTERRUPT_TIMER1{} 

typedef struct
{
	char DeltaX, DeltaY;
} vector;

typedef struct
{
	char Plus90, Minus90;	/* pointer to shape rotated +/- 90 degrees */
	char Color;		/* shape color */
	vector Dir[4];
} shape;	/* drawing instructions for this shape */

const shape Shapes[]=
{
	/* shape #0:			cube */
	{ 0, 0, COLOR_BLUE, { DIR_UP, DIR_RT, DIR_DN, DIR_LT }},
	/* shapes #1 & #2:		bar */
	{ 2, 2, COLOR_GREEN, { DIR_LT, DIR_RT, DIR_RT, DIR_RT }},
	{ 1, 1, COLOR_GREEN, { DIR_UP, DIR_DN, DIR_DN, DIR_DN }},
	/* shapes #3 & #4:		'Z' shape */
	{ 4, 4, COLOR_CYAN, { DIR_LT, DIR_RT, DIR_DN, DIR_RT }},
	{ 3, 3, COLOR_CYAN, { DIR_UP, DIR_DN, DIR_LT, DIR_DN }},
	/* shapes #5 & #6:		'S' shape */
	{ 6, 6, COLOR_RED, { DIR_RT, DIR_LT, DIR_DN, DIR_LT }},
	{ 5, 5, COLOR_RED, { DIR_UP, DIR_DN, DIR_RT, DIR_DN }},
	/* shapes #7, #8, #9, #10:	'J' shape */
	{ 8, 10, COLOR_MAGENTA, { DIR_RT, DIR_LT, DIR_LT, DIR_UP }},
	{ 9, 7, COLOR_MAGENTA, { DIR_UP, DIR_DN, DIR_DN, DIR_LT }},
	{ 10, 8, COLOR_MAGENTA, { DIR_LT, DIR_RT, DIR_RT, DIR_DN }},
	{ 7, 9, COLOR_MAGENTA, { DIR_DN, DIR_UP, DIR_UP, DIR_RT }},
	/* shapes #11, #12, #13, #14:	'L' shape */
	{ 12, 14, COLOR_YELLOW, { DIR_RT, DIR_LT, DIR_LT, DIR_DN }},
	{ 13, 11, COLOR_YELLOW, { DIR_UP, DIR_DN, DIR_DN, DIR_RT }},
	{ 14, 12, COLOR_YELLOW, { DIR_LT, DIR_RT, DIR_RT, DIR_UP }},
	{ 11, 13, COLOR_YELLOW, { DIR_DN, DIR_UP, DIR_UP, DIR_LT }},
	/* shapes #15, #16, #17, #18:	'T' shape */
	{ 16, 18, COLOR_WHITE, { DIR_UP, DIR_DN, DIR_LT, DIR_RT2 }},
	{ 17, 15, COLOR_WHITE, { DIR_LT, DIR_RT, DIR_UP, DIR_DN2 }},
	{ 18, 16, COLOR_WHITE, { DIR_DN, DIR_UP, DIR_RT, DIR_LT2 }},
	{ 15, 17, COLOR_WHITE, { DIR_RT, DIR_LT, DIR_DN, DIR_UP2 }}
};

/*This is where most of the memory is used!  In order to save
memory, one byte is used to represent two characters in the
playing area.  Three bits are used to store the color, and one bit
is used as a redraw flag. The functions wscr and rscr below make
this easier to handle.  Check that you have more than 30 bytes
available for stack for the program to run properly (the mem
or map files)*/

idata unsigned char Screen[(SCN_WID+1)/2][SCN_HT];

/* Games are more fun with levels and score! */
unsigned int Level=0;
unsigned int Score=0;

void wscr (unsigned char x, unsigned char y, unsigned char val)
{
	unsigned char j;
	j=Screen[x/2][y];
	if((x&1)==0)
	{
		j&=0xf0;
		Screen[x/2][y]=(j|(val&0x7)|(val&0x80?8:0));
	}
	else
	{
		j&=0xf;
		Screen[x/2][y]=j|((val*0x10)&0x70)|(val&0x80);
	}
}

unsigned char rscr (unsigned char x, unsigned char y)
{
	unsigned char j;
	j=Screen[x/2][y];
	if(x&1) j/=0x10;
	return ((j&0x7)|(j&0x8?0x80:0));
}



void putchar(char c)
{
	if (c=='\n')
	{
		while (!TI);
		TI=0;
		SBUF='\r';
	}
	while (!TI);
	TI=0;
	SBUF=c;
}

/* ////////////////////////////////////////////////////////////////////////////
	ANSI GRAPHIC OUTPUT 
//////////////////////////////////////////////////////////////////////////// */

/*****************************************************************************
	name: refresh
	updates display device based on contents of global
	char array Screen[][]. Updates only those boxes
	marked for change
*****************************************************************************/
void refresh(void)
{
	char XPos, YPos;

	for(YPos=0; YPos < SCN_HT; YPos++)
	{
		for(XPos=0; XPos < SCN_WID; XPos++)
		{
			if((rscr(XPos, YPos)&0x80)==0x80)
			{
				wscr(XPos, YPos, rscr(XPos, YPos)&0x7f);
				/* 0xDB is a solid rectangular block in the PC character set */
				printf(GOTO_YX, YPos + 1, (XPos*2)+1);/* gotoxy(XPos, YPos) */
				/*Two characters are printed, so the block looks like a square*/
				printf("\x1B[3%dm\xDB\xDB", rscr(XPos, YPos));
			}
		}
	}
	/* Foreground black, Background white */
	printf(BKF_WTB);
}
	
/* ////////////////////////////////////////////////////////////////////////////
			GRAPHIC CHUNK DRAW & HIT DETECT
//////////////////////////////////////////////////////////////////////////// */

/*****************************************************************************
	name:	blockDraw
	action:	draws one graphic block in display buffer at
		position (XPos, YPos)
*****************************************************************************/
void blockDraw(char XPos, char YPos, unsigned char Color)
{
	if(XPos >= SCN_WID) XPos=SCN_WID - 1;
	if(YPos >= SCN_HT) YPos=SCN_HT - 1;

	wscr(XPos, YPos, Color|0x80);
}

/*****************************************************************************
	name:	blockHit
	action:	determines if coordinates (XPos, YPos) are already
		occupied by a graphic block
	returns:color of graphic block at (XPos, YPos) (zero if black/
		empty)
*****************************************************************************/
char blockHit(char XPos, char YPos)
{
	return(rscr(XPos, YPos)&0x7f);
}

/* ////////////////////////////////////////////////////////////////////////////
			SHAPE DRAW & HIT DETECT
//////////////////////////////////////////////////////////////////////////// */

/*****************************************************************************
	name:	shapeDraw
	action:	draws shape WhichShape in display buffer at
		position (XPos, YPos)
*****************************************************************************/
void shapeDraw(char XPos, char YPos, char WhichShape)
{
	char Index;

	for(Index=0; Index < 4; Index++)
	{
		blockDraw(XPos, YPos, Shapes[WhichShape].Color);
		XPos += Shapes[WhichShape].Dir[Index].DeltaX;
		YPos += Shapes[WhichShape].Dir[Index].DeltaY;
	}
	blockDraw(XPos, YPos, Shapes[WhichShape].Color);
}

/*****************************************************************************
	name:	shapeErase
	action:	erases shape WhichShape in display buffer at
		position (XPos, YPos) by setting its color to zero
*****************************************************************************/
void shapeErase(char XPos, char YPos, char WhichShape)
{
	char Index;

	for(Index=0; Index < 4; Index++)
	{
		blockDraw(XPos, YPos, COLOR_BLACK);
		XPos += Shapes[WhichShape].Dir[Index].DeltaX;
		YPos += Shapes[WhichShape].Dir[Index].DeltaY;
	}
	blockDraw(XPos, YPos, COLOR_BLACK);
}

/*****************************************************************************
	name:	shapeHit
	action:	determines if shape WhichShape would collide with
		something already drawn in display buffer if it
		were drawn at position (XPos, YPos)
	returns:nonzero if hit, zero if nothing there
*****************************************************************************/
char shapeHit(char XPos, char YPos, char WhichShape)
{
	char Index;

	for(Index=0; Index < 4; Index++)
	{
		if(blockHit(XPos, YPos)) return(1);
		XPos += Shapes[WhichShape].Dir[Index].DeltaX;
		YPos += Shapes[WhichShape].Dir[Index].DeltaY;
	}
	if(blockHit(XPos, YPos)) return(1);
	return(0);
}

/* //////////////////////////////////////////////////////////////////////////
			MAIN ROUTINES
////////////////////////////////////////////////////////////////////////// */

/***************************************************************************
	name:	screenInit
	action:	clears display buffer, marks all rows dirty,
		set raw keyboard mode
***************************************************************************/
void screenInit(void)
{
    unsigned char XPos, YPos;

	for(YPos=0; YPos < SCN_HT; YPos++)
	{
		for(XPos=1; XPos < (SCN_WID - 1); XPos++) wscr(XPos,YPos,0x80);
		/*The blue sides*/
		wscr(0, YPos, COLOR_BLUE|0x80);
		wscr(SCN_WID - 1, YPos, COLOR_BLUE|0x80);
	}
	for(XPos=0; XPos < SCN_WID; XPos++)
	{
		/*Blue top and botton*/
		wscr(XPos, 0, COLOR_BLUE|0x80);
		wscr(XPos, SCN_HT-1, COLOR_BLUE|0x80);
	}
}

void collapse(void)
{
    char SolidRows;
	char Row, Col, Temp;
	code unsigned int bonus[]={0, 50, 100, 200, 400 };

    /* Determine which rows are solidly filled */
	SolidRows=0;
	for(Row=1; Row < SCN_HT - 1; Row++)
	{
		Temp=0;
		for(Col=1; Col < SCN_WID - 1; Col++)
			if(rscr(Col, Row)&0x7f) Temp++;
		if(Temp == SCN_WID - 2)
		{
		    /* Use the redraw bit of column zero to mark a solid row */
		    wscr(0, Row, COLOR_BLUE|0x80);
			SolidRows++;
			Level++;
		}
	}
	if(SolidRows == 0) return;

	Score+=bonus[SolidRows]; /* Bonus! */
	
    /* Collapse the solid rows */
	for(Temp=Row=SCN_HT - 2; Row > 0; Row--, Temp--)
	{
		while(rscr(0, Temp)&0x80) Temp--;
		if(Temp < 1)
		{	
			for(Col=1; Col < SCN_WID - 1; Col++)
				wscr(Col, Row, COLOR_BLACK|0x80);
		}
		else
		{	
			for(Col=1; Col < SCN_WID - 1; Col++)
				wscr(Col, Row, rscr(Col,Temp)|0x80);
		}
	}
	refresh();
}

char getKey(void)
{
	if(!RI) return 0;
	
	RI=0;
	switch(toupper(SBUF))
	{
		case 'Q': return KEY_QUIT;
		case 'K': return KEY_CCW;
		case 'U': return KEY_CW;
		case 'J': return KEY_LEFT;
		case 'L': return KEY_RIGHT;
		case 'I': return KEY_UP;
		case ',':
		case 'M': return KEY_DOWN;
		case 'B': return KEY_BEGIN;
		case 'P':
			while(!RI);
			RI=0;
		default:
		break;
	}
	return 0;
}

void wastetime(int j)
{
	unsigned char k;
	while((j--)&&(RI==0))
	{
		for(k=0; k<delay; k++) if (RI) break;
	}
}

void exit (void)
{
    printf(CLEAR_SCREEN CURSOR_ON BKF_WTB);
    _asm
       lcall 1bh
    _endasm;
    while(1);
}


void InitADC (void)
{
	SFRPAGE = 0x00;
	ADEN=0; // Disable ADC
	
	ADC0CN1=
		(0x2 << 6) | // 0x0: 10-bit, 0x1: 12-bit, 0x2: 14-bit
        (0x0 << 3) | // 0x0: No shift. 0x1: Shift right 1 bit. 0x2: Shift right 2 bits. 0x3: Shift right 3 bits.		
		(0x0 << 0) ; // Accumulate n conversions: 0x0: 1, 0x1:4, 0x2:8, 0x3:16, 0x4:32
	
	ADC0CF0=
	    ((SYSCLK/SARCLK) << 3) | // SAR Clock Divider. Max is 18MHz. Fsarclk = (Fadcclk) / (ADSC + 1)
		(0x0 << 2); // 0:SYSCLK ADCCLK = SYSCLK. 1:HFOSC0 ADCCLK = HFOSC0.
	
	ADC0CF1=
		(0 << 7)   | // 0: Disable low power mode. 1: Enable low power mode.
		(0x1E << 0); // Conversion Tracking Time. Tadtk = ADTK / (Fsarclk)
	
	ADC0CN0 =
		(0x0 << 7) | // ADEN. 0: Disable ADC0. 1: Enable ADC0.
		(0x0 << 6) | // IPOEN. 0: Keep ADC powered on when ADEN is 1. 1: Power down when ADC is idle.
		(0x0 << 5) | // ADINT. Set by hardware upon completion of a data conversion. Must be cleared by firmware.
		(0x0 << 4) | // ADBUSY. Writing 1 to this bit initiates an ADC conversion when ADCM = 000. This bit should not be polled to indicate when a conversion is complete. Instead, the ADINT bit should be used when polling for conversion completion.
		(0x0 << 3) | // ADWINT. Set by hardware when the contents of ADC0H:ADC0L fall within the window specified by ADC0GTH:ADC0GTL and ADC0LTH:ADC0LTL. Can trigger an interrupt. Must be cleared by firmware.
		(0x0 << 2) | // ADGN (Gain Control). 0x0: PGA gain=1. 0x1: PGA gain=0.75. 0x2: PGA gain=0.5. 0x3: PGA gain=0.25.
		(0x0 << 0) ; // TEMPE. 0: Disable the Temperature Sensor. 1: Enable the Temperature Sensor.

	ADC0CF2= 
		(0x0 << 7) | // GNDSL. 0: reference is the GND pin. 1: reference is the AGND pin.
		(0x1 << 5) | // REFSL. 0x0: VREF pin (external or on-chip). 0x1: VDD pin. 0x2: 1.8V. 0x3: internal voltage reference.
		(0x1F << 0); // ADPWR. Power Up Delay Time. Tpwrtime = ((4 * (ADPWR + 1)) + 2) / (Fadcclk)
	
	ADC0CN2 =
		(0x0 << 7) | // PACEN. 0x0: The ADC accumulator is over-written.  0x1: The ADC accumulator adds to results.
		(0x0 << 0) ; // ADCM. 0x0: ADBUSY, 0x1: TIMER0, 0x2: TIMER2, 0x3: TIMER3, 0x4: CNVSTR, 0x5: CEX5, 0x6: TIMER4, 0x7: TIMER5, 0x8: CLU0, 0x9: CLU1, 0xA: CLU2, 0xB: CLU3

	ADEN=1; // Enable ADC
}

void InitPinADC (unsigned char portno, unsigned char pin_num)
{
	unsigned char mask;
	
	mask=1<<pin_num;

	SFRPAGE = 0x20;
	switch (portno)
	{
		case 0:
			P0MDIN &= (~mask); // Set pin as analog input
			P0SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		case 1:
			P1MDIN &= (~mask); // Set pin as analog input
			P1SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		case 2:
			P2MDIN &= (~mask); // Set pin as analog input
			P2SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		default:
		break;
	}
	SFRPAGE = 0x00;
}

unsigned int ADC_at_Pin(unsigned char pin)
{
	ADC0MX = pin;   // Select input from pin
	ADINT = 0;
	ADBUSY = 1;     // Convert voltage at the pin
	while (!ADINT); // Wait for conversion to complete
	return (ADC0);
}

float Volts_at_Pin(unsigned char pin)
{
	 return ((ADC_at_Pin(pin)*VDD)/16383.0);
}

int getsn (char * buff, int len)
{
	int j;
	char c;
	
	for(j=0; j<(len-1); j++)
	{
		c=getchar();
		if ( (c=='\n') || (c=='\r') )
		{
			buff[j]=0;
			return j;
		}
		else
		{
			buff[j]=c;
		}
	}
	buff[j]=0;
	return len;
}

unsigned char overflow_count;

void Timer2_ISR (void) interrupt INTERRUPT_TIMER2
{
	TF2H = 0; // Clear Timer2 interrupt flag
	OUT0=!OUT0;
	OUT1=!OUT0;
}

// Uses Timer4 to delay <ms> mili-seconds. 
void Timer4ms(unsigned char ms)
{
	unsigned char i;// usec counter
	unsigned char k;
	
	k=SFRPAGE;
	SFRPAGE=0x10;
	// The input for Timer 4 is selected as SYSCLK by setting bit 0 of CKCON1:
	CKCON1|=0b_0000_0001;
	
	TMR4RL = 65536-(SYSCLK/1000L); // Set Timer4 to overflow in 1 ms.
	TMR4 = TMR4RL;                 // Initialize Timer4 for first overflow
	
	TF4H=0; // Clear overflow flag
	TR4=1;  // Start Timer4
	for (i = 0; i < ms; i++)       // Count <ms> overflows
	{
		while (!TF4H);  // Wait for overflow
		TF4H=0;         // Clear overflow indicator
	}
	TR4=0; // Stop Timer4
	SFRPAGE=k;	
}

void I2C_write (unsigned char output_data)
{
	SMB0DAT = output_data; // Put data into buffer
	SI = 0;
	while (!SI); // Wait until done with send
}

unsigned char I2C_read (void)
{
	unsigned char input_data;

	SI = 0;
	while (!SI); // Wait until we have data to read
	input_data = SMB0DAT; // Read the data

	return input_data;
}

void I2C_start (void)
{
	ACK = 1;
	STA = 1;     // Send I2C start
	STO = 0;
	SI = 0;
	while (!SI); // Wait until start sent
	STA = 0;     // Reset I2C start
}

void I2C_stop(void)
{
	STO = 1;  	// Perform I2C stop
	SI = 0;	// Clear SI
	//while (!SI);	   // Wait until stop complete (Doesn't work???)
}

void nunchuck_init(bit print_extension_type)
{
	unsigned char i, buf[6];
	
	// Newer initialization format that works for all nunchucks
	I2C_start();
	I2C_write(0xA4);
	I2C_write(0xF0);
	I2C_write(0x55);
	I2C_stop();
	Timer4ms(1);
	 
	I2C_start();
	I2C_write(0xA4);
	I2C_write(0xFB);
	I2C_write(0x00);
	I2C_stop();
	Timer4ms(1);

	// Read the extension type from the register block.  For the original Nunchuk it should be
	// 00 00 a4 20 00 00.
	I2C_start();
	I2C_write(0xA4);
	I2C_write(0xFA); // extension type register
	I2C_stop();
	Timer4ms(3); // 3 ms required to complete acquisition

	I2C_start();
	I2C_write(0xA5);
	
	// Receive values
	for(i=0; i<6; i++)
	{
		buf[i]=I2C_read();
	}
	ACK=0;
	I2C_stop();
	Timer4ms(3);
	
	if(print_extension_type)
	{
		printf("Extension type: %02x  %02x  %02x  %02x  %02x  %02x\n", 
			buf[0],  buf[1], buf[2], buf[3], buf[4], buf[5]);
	}

	// Send the crypto key (zeros), in 3 blocks of 6, 6 & 4.

	I2C_start();
	I2C_write(0xA4);
	I2C_write(0xF0);
	I2C_write(0xAA);
	I2C_stop();
	Timer4ms(1);

	I2C_start();
	I2C_write(0xA4);
	I2C_write(0x40);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_stop();
	Timer4ms(1);

	I2C_start();
	I2C_write(0xA4);
	I2C_write(0x40);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_stop();
	Timer4ms(1);

	I2C_start();
	I2C_write(0xA4);
	I2C_write(0x40);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_write(0x00);
	I2C_stop();
	Timer4ms(1);
}

void nunchuck_getdata(unsigned char * s)
{
	unsigned char i;

	// Start measurement
	I2C_start();
	I2C_write(0xA4);
	I2C_write(0x00);
	I2C_stop();
	Timer4ms(3); 	// 3 ms required to complete acquisition

	// Request values
	I2C_start();
	I2C_write(0xA5);
	
	// Receive values
	for(i=0; i<6; i++)
	{
		s[i]=(I2C_read()^0x17)+0x17; // Read and decrypt
	}
	ACK=0;
	I2C_stop();
}

void main (void)
{
	char rbuf[6];
 	int joy_x, joy_y, off_x, off_y, acc_x, acc_y, acc_z;
 	bit but1, but2;
 	long int x, f;
 	float v, temperature;
 	xdata char buff[17];
 	int TEMP_flag = 0;
	xdata char Fell, NewShape, NewX, NewY;
	xdata char Shape, X, Y;
	xdata char Key;

	/*-------------------------------------TETRIS OUTPUT---------------------------------*/
	#define TEXT_POS (SCN_WID*2+2)
    /* Banner screen */
	printf_tiny(CLEAR_SCREEN CURSOR_OFF);
	printf_tiny(GOTO_YX "TETRIS by Alexei Pazhitnov", 1, TEXT_POS);
	printf_tiny(GOTO_YX "Originally by Chris Giese", 2, TEXT_POS);
	printf_tiny(GOTO_YX "8052/C51 port by Jesus Calvino-Fraga", 3, TEXT_POS);
	printf_tiny(GOTO_YX "'K':Rotate, 'P':Pause, 'Q':Quit", 5, TEXT_POS);
	printf_tiny(GOTO_YX "'J':Left, 'L':Right, 'M':Down", 6, TEXT_POS);
	screenInit();
	refresh();
NEW_GAME:
	printf_tiny(BKF_WTB GOTO_YX "Press 'B' to begin", 8, TEXT_POS);
	do
	{
		Key=getKey();
		if(Key==KEY_QUIT) exit();
	} while (Key!=KEY_BEGIN);
	screenInit();
	
	Level=1;
	Score=0;
	printf_tiny(BKF_WTB GOTO_YX CLR_TO_END_LINE, 8, TEXT_POS);
	goto NEW_SHAPE;

	while(1)
	{
	    Fell=0;
		NewShape=Shape;
		NewX=X;
		NewY=Y;
		Key=getKey();
		if(Key == 0)
		{
		    NewY++;
			Fell=1;
			/*Level 42 is pretty hard already, so set it as the limit*/
			wastetime(15000-((Level<42?Level:42)*300));
		}
		
		if(RI) Key=getKey();
		
		if(Key != 0)
		{
			NewY=Y;
		    if(Key == KEY_QUIT) break;
			if(Key == KEY_CCW)
				NewShape=Shapes[Shape].Plus90;
			else if(Key == KEY_CW)
				NewShape=Shapes[Shape].Minus90;
			else if(Key == KEY_LEFT)
			{	if(X) NewX=X - 1; }
			else if(Key == KEY_RIGHT)
			{	if(X < SCN_WID - 1) NewX=X + 1; }
            /*else if(Key == KEY_UP)
			{	if(Y) NewY=Y - 1; } 	cheat */
			else if(Key == KEY_DOWN)
			{	if(Y < SCN_HT - 1) NewY=Y + 1; }
			Fell=0;
		}
        /* If nothing has changed, skip the bottom half of this loop */
		if((NewX == X) && (NewY == Y) && (NewShape == Shape))
			continue;
        /* Otherwise, erase old shape from the old pos'n */
		shapeErase(X, Y, Shape);
        /* Hit anything? */
		if(shapeHit(NewX, NewY, NewShape) == 0) /* no, update pos'n */
		{
		    X=NewX;
			Y=NewY;
			Shape=NewShape;
		}
		else if(Fell) /* Yes -- did the piece hit something while falling on its own? */
		{
    		shapeDraw(X, Y, Shape); /* Yes, draw it at the old pos'n... */
            /* ... and spawn new shape */
NEW_SHAPE:
			Y=3;
			X=SCN_WID / 2;
			Shape=TL0 % 19; //rand() was here, use timer 0 register instead...
			collapse();
            /* If newly spawned shape hits something, game over */
			if(shapeHit(X, Y, Shape))
			{
			    printf(BKF_WTB GOTO_YX " GAME OVER ", SCN_HT/2, (SCN_WID-5));
				goto NEW_GAME;
			}
			Score+=Level;
			printf(GOTO_YX CLR_TO_END_LINE "Level: %u", 15, TEXT_POS, Level);
			printf(GOTO_YX CLR_TO_END_LINE "Score: %u", 16, TEXT_POS, Score);
		}
        /* Hit something because of user movement/rotate OR no hit: just redraw it */
		shapeDraw(X, Y, Shape);
		refresh();
	}
    exit();
/*---------------------------------------TETRIS END-----------------------------------------------*/
	
	InitPinADC(1, 6); // Configure P1.6 as analog input
    InitADC();
 	
 	// Configure the LCD
	LCD_4BIT();
 	
	printf("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
	printf("\n\nEFM8LB1 WII Nunchuck I2C Reader\n");

	Timer4ms(200);
	nunchuck_init(1);
	Timer4ms(100);

	nunchuck_getdata(rbuf);

	off_x=(int)rbuf[0]-128;
	off_y=(int)rbuf[1]-128;
	printf("Offset_X:%4d Offset_Y:%4d\n\n", off_x, off_y);
	
	f = 16275;    

	while(1)
	{
		nunchuck_getdata(rbuf);

		joy_x=(int)rbuf[0]-128-off_x;
		joy_y=(int)rbuf[1]-128-off_y;
		acc_x=rbuf[2]*4; 
		acc_y=rbuf[3]*4;
		acc_z=rbuf[4]*4;

		but1=(rbuf[5] & 0x01)?1:0;
		but2=(rbuf[5] & 0x02)?1:0;
		if (rbuf[5] & 0x04) acc_x+=2;
		if (rbuf[5] & 0x08) acc_x+=1;
		if (rbuf[5] & 0x10) acc_y+=2;
		if (rbuf[5] & 0x20) acc_y+=1;
		if (rbuf[5] & 0x40) acc_z+=2;
		if (rbuf[5] & 0x80) acc_z+=1;
		
		if(joy_y > 90)
		{
			printf ("Move forward\x1b[0J\r");
			LCDprint("Move forward", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			TR2 = 1;
			waitms(52);	
		}
		
		else if(joy_y < -90){
			printf("Move backward\x1b[0J\r");
			LCDprint("Move backward", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			waitms(100);
			TR2 = 1;
			waitms(52);
		}
		else if(joy_x < -90){
			printf("Left\x1b[0J\r");
			LCDprint("Left", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			TR2 = 1;
			waitms(52);
		}

		else if(joy_x > 90){
			printf("Right\x1b[0J\r");
			LCDprint("Right", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			waitms(100);
			waitms(100);
			TR2 = 1;
			waitms(52);
		}
		
		else if(but1 == 0)
		{
			printf("Switch Mode\x1b[0J\r");
			LCDprint("Switch Mode", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			TR2 = 1;
			waitms(52);
			waitms(100);
			waitms(100);
		}
		
		
		else if(but2 == 0)
		{
			printf("Changing Dis\x1b[0J\r");
			LCDprint("Changing Dis", 1, 1);
			TR2=0;
			OUT1 = 0;
			OUT0 = 0;
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			waitms(100);
			TR2 = 1;
			waitms(52);
		}
		
		else if(SOUND == 0)
		{
			LOOP_B:
			if(SOUND == 0)
			{
				printf ("Play Sound");
				LCDprint("Play Sound", 1, 1);
				TR2=0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				TR2 = 1;
				waitms(52);
				goto LOOP_B;
			}	
		}
		
		else if(TEMP == 0)
		{
			TEMP_flag = 1;
			while(TEMP_flag == 1)
			{
				if(TEMP == 0)
					TEMP_flag = 0;
				LCDprint("Temperature", 1, 1);
				v = Volts_at_Pin(QFP32_MUX_P1_6);
				temperature = 100*(v - 2.73);
				printf ("temperature=%7.5f, v=%f\n", temperature, v);
				LCDprint("temperature:", 1, 1);
				sprintf(buff, "%f", temperature);
				LCDprint(buff, 2, 1);
				waitms(500);
			}
			
		}
		
		else
		{
			LCDprint("Wait Command", 1, 1);
			LCDprint(" ", 2, 1);
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

   }
}

/*
void main (void)
{
	unsigned long int x, f;
	int test_press = 1;
	float v;
	//float vtest;
	int temp_flag = 0;
	float temperature;
	char Fell, NewShape, NewX, NewY;
	char Shape, X, Y;
	char Key;
	
	
	
	
    
    //-------------------------------------Begin Main Portion---------------------------------
    // Configure the LCD
	LCD_4BIT();
	
	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.
	printf("Variable frequency generator for the EFM8LB1.\r\n"
	       "Check pins P2.0 and P2.1 with the oscilloscope.\r\n");
	       
	//printf("New frequency=");
	//scanf("%lu \n", &f);
	//printf("\n");       
	f = 15920;

	while(1)
	{
		TEMP:
		while(temp_flag != 0)
		{	
			if(DISPLAY == 0)
			{
				waitms(80);
				LOOP_OFFF:
				if(DISPLAY == 0)
				{
					waitms(80);
					temp_flag = 0;
					goto LOOP_OFFF;
				}	
				goto DISPLAY_BUFFER;
			}
			v = Volts_at_Pin(QFP32_MUX_P1_6);
			//temperature = 100*(v - 2.73);
			temperature = v*110.0 + 22;
			printf ("temperature=%7.5f, v=%f\n", temperature, v);
			LCDprint("temperature:", 1, 1);
			sprintf(buff, "%f", temperature);
			LCDprint(buff, 2, 1);
			waitms(500);
		}	
		DISPLAY_BUFFER:
		LCDprint("Wait Command", 1, 1);
		LCDprint(" ", 2, 1);

		FREQ:
		if(DISPLAY == 0)
		{
			waitms(80);
			LOOP_A:
			if(DISPLAY == 0)
			{
				waitms(80);
				temp_flag = 1;
				goto LOOP_A;
			}	
			goto TEMP;
		}

		if(SWITCHER == 0)
		{
			waitms(80);
			LOOP_F:
			if(SWITCHER == 0)
			{
				printf("Switch Modes");
				LCDprint("Switch Modes",1,1);
				LCDprint(" ", 2, 1);
				waitms(80);
				goto LOOP_F;

			}
				TR2 = 0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				TR2 = 1;
				waitms(52);
		}

		if(FORWARD == 0)
		{
			LOOP_B:
			if(FORWARD == 0)
			{
				printf ("Move forward");
				LCDprint("Move forward", 1, 1);
				TR2=0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				waitms(100);
				TR2 = 1;
				waitms(52);
				goto LOOP_B;
			}	
		}
		
		if(BACKWARD == 0)
		{	
			LOOP_C:
			if(BACKWARD == 0)
			{
				printf ("Move backward");
				LCDprint("Move backward", 1, 1);
				TR2=0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				waitms(100);
				waitms(100);
				TR2 = 1;
				waitms(52);
				goto LOOP_C;
			}	
		}
		
		if(RIGHT == 0)
		{	
			LOOP_D:
			if(RIGHT == 0)
			{
				printf ("Move right");
				LCDprint("Move right", 1, 1);
				TR2=0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				TR2 = 1;
				waitms(52);
				goto LOOP_D;
			}
		}
		
		if(LEFT == 0)
		{
			LOOP_E:
			if(LEFT == 0)
			{
				printf ("Move left");
				LCDprint("Move left", 1, 1);
				TR2=0;
				OUT1 = 0;
				OUT0 = 0;
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				waitms(100);
				TR2 = 1;
				waitms(52);
				goto LOOP_E;
			}	
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
	}
}
*/