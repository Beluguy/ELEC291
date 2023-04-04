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

#include <stdio.h>
#include <EFM8LB1.h>
#include <ctype.h>
#include "globals.h"

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
