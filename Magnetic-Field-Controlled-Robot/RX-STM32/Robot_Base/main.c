#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"
#include "adc.h"
#include "macros.h"
#include "../LCD/lcd.h"
#include "pitches.h"

#define SPKR_F 520L
#define ZERO_TOL 800L
#define LTHRESH1 2100
#define LTHRESH2 870
#define RTHRESH1 2050
#define RTHRESH2 940

int readings[4];

int melody[] = {

  //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=192
  
  NOTE_E6, 4,  NOTE_B5,8,  NOTE_C6,8,  NOTE_D6,4,  NOTE_C6,8,  NOTE_B5,8,
  NOTE_A5, 4,  NOTE_A5,8,  NOTE_C6,8,  NOTE_E6,4,  NOTE_D6,8,  NOTE_C6,8,
  NOTE_B5, -4,  NOTE_C6,8,  NOTE_D6,4,  NOTE_E6,4,
  NOTE_C6, 4,  NOTE_A5,4,  NOTE_A5,8,  NOTE_A5,4,  NOTE_B5,8,  NOTE_C6,8,

  NOTE_D6, -4,  NOTE_F5,8,  NOTE_A6,4,  NOTE_G5,8,  NOTE_F5,8,
  NOTE_E6, -4,  NOTE_C6,8,  NOTE_E6,4,  NOTE_D6,8,  NOTE_C6,8,
  NOTE_B5, 4,  NOTE_B5,8,  NOTE_C6,8,  NOTE_D6,4,  NOTE_E6,4,
  NOTE_C6, 4,  NOTE_A5,4,  NOTE_A5,4, REST, 4,

  NOTE_E6, 4,  NOTE_B5,8,  NOTE_C6,8,  NOTE_D6,4,  NOTE_C6,8,  NOTE_B5,8,
  NOTE_A5, 4,  NOTE_A5,8,  NOTE_C6,8,  NOTE_E6,4,  NOTE_D6,8,  NOTE_C6,8,
  NOTE_B5, -4,  NOTE_C6,8,  NOTE_D6,4,  NOTE_E6,4,
  NOTE_C6, 4,  NOTE_A5,4,  NOTE_A5,8,  NOTE_A5,4,  NOTE_B5,8,  NOTE_C6,8,

  NOTE_D6, -4,  NOTE_F5,8,  NOTE_A6,4,  NOTE_G5,8,  NOTE_F5,8,
  NOTE_E6, -4,  NOTE_C6,8,  NOTE_E6,4,  NOTE_D6,8,  NOTE_C6,8,
  NOTE_B5, 4,  NOTE_B5,8,  NOTE_C6,8,  NOTE_D6,4,  NOTE_E6,4,
  NOTE_C6, 4,  NOTE_A5,4,  NOTE_A5,4, REST, 4,
  

  NOTE_E6,2,  NOTE_C6,2,
  NOTE_D6,2,   NOTE_B5,2,
  NOTE_C6,2,   NOTE_A5,2,
  NOTE_GS5,2,  NOTE_B5,4,  REST,8, 
  NOTE_E6,2,   NOTE_C6,2,
  NOTE_D6,2,   NOTE_B5,2,
  NOTE_C6,4,   NOTE_E6,4,  NOTE_A6,2,
  NOTE_GS6,2,

};

// change this to make the song slower or faster
#define TEMPO 300
// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms (60s/tempo)*4 beats
int wholenote = (60000 * 4) / TEMPO;

int divider = 0, noteDuration = 0;

volatile int Count10 = 0;
volatile int Count = 0;

volatile char start_read;
volatile char togglethresh = 0;
volatile char toggle = 0;
int lthresh = LTHRESH1;
int rthresh = RTHRESH1;

// LQFP32 pinout
//                 ----------
//           VDD -|1       32|- VSS
//          PC14 -|2       31|- BOOT0
//          PC15 -|3       30|- PB7 (OUT 5)
//          NRST -|4       29|- PB6 (RF)
//          VDDA -|5       28|- PB5 (RB)
// LCD_RS    PA0 -|6       27|- PB4 (LF)
// LCD_E     PA1 -|7       26|- PB3 (LB)
// LCD_D4    PA2 -|8       25|- PA15 (buzzer)
// LCD_D5    PA3 -|9       24|- PA14 (push button)
// LCD_D6    PA4 -|10      23|- PA13
// LCD_D7    PA5 -|11      22|- PA12 (pwm2)
// 		 	 PA6 -|12      21|- PA11 (pwm1)
//        	 PA7 -|13      20|- PA10 (Reserved for RXD)
// R input 	 PB0 -|14      19|- PA9  (Reserved for TXD)
// L input	 PB1 -|15      18|- PA8  (Measure the period at this pin) unused
//           VSS -|16      17|- VDD
//                 ----------

//  ----------------------------------------- GLOBAL VARS ------------------------------------------------------------

// Interrupt service routines are the same as normal
// subroutines (or C funtions) in Cortex-M microcontrollers.
// The following should happen at a rate of 1kHz.
// The following function is associated with the TIM21 interrupt 
// via the interrupt vector table defined in startup.s
void TIM21_Handler(void) 
{
	TIM21->SR &= ~BIT0; // clear update interrupt flag
    Count10++;

    if (Count10 > 10) {
        Count10=0;
        if (readADC(ADC_CHSELR_CHSEL8) < ZERO_TOL && start_read == 0) {
            start_read = 1;
            OffCycles++;
        }
    }

    if (start_read == 1) {
	    Count++;
    }

    if (Count == 85) {
        readings[0] = readADC(ADC_CHSELR_CHSEL8);
    } else if (Count == 90) {
        readings[1] = readADC(ADC_CHSELR_CHSEL8);
    } else if (Count == 95) {
        readings[2] = readADC(ADC_CHSELR_CHSEL8);
    } else if (Count > 100) { // every 100ms
        readings[3] = readADC(ADC_CHSELR_CHSEL8);
        //printf("%d,%d,%d,%d \r\n", readings[0],readings[1],readings[2],readings[3]);
        Count = 0;
        int average = 0;
        for (int i = 0; i < 4; i++) {
            average += readings[i];
        }
        average = average * 0.25;
        if (average < ZERO_TOL)
        {
            OffCycles++;
        } else {
            switch (OffCycles) {
            case 0:
                PB6_1;
                PB5_1;
                PB4_1;
                PB3_1;
                start_read = 0;
                break;
            case 1:
                // fwd
                puts("fwd\r\n");
                PB6_0;
                PB5_1;
                PB4_0;
                PB3_1;
                break;
            case 2:
                // back
                puts("back\r\n");
                PB6_1;
                PB5_0;
                PB4_1;
                PB3_0;
                break;
            case 3:
                // right
                puts("right\r\n");
                PB6_1;
                PB5_0;
                PB4_0;
                PB3_1;
                break;
            case 4:
                // left
                puts("left\r\n");
                PB6_0;
                PB5_1;
                PB4_1;
                PB3_0;
                break;
            case 5:
                puts("toggle\r\n");
                toggle = 1;
                break;
            case 6:
                puts("following dist\r\n");
                togglethresh = 1;
                break;
            case 7:
                // tetris 
                PB6_1;
                PB5_1;
                PB4_1;
                PB3_1;
                puts("tetris\r\n");
                LCDprint("Tetris",2,1);
                for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
                {

                    // calculates the duration of each note
                    divider = melody[thisNote + 1];
                    if (divider > 0)
                    {
                        // regular note, just proceed
                        noteDuration = (wholenote) / divider;
                    }
                    else if (divider < 0)
                    {
                        // dotted notes are represented with negative durations!!
                        noteDuration = (wholenote) / abs(divider);
                        noteDuration *= 1.5; // increases the duration in half for dotted notes
                    }

                    // we only play the note for 90% of the duration, leaving 10% as a pause
                    tone(melody[thisNote], noteDuration * 0.9);

                    // Wait for the specief duration before playing the next note.
                    waitms(noteDuration);
                }
                break;
            default:
                PB6_1;
                PB5_1;
                PB4_1;
                PB3_1;
                start_read = 0;
                break;
            }
            OffCycles = 0;
        }
    }
    return;
}

void Hardware_Init(void)
{
	RCC->IOPENR  |= (BIT1|BIT0);         // peripheral clock enable for ports A and B

    GPIOA->OSPEEDR=0xffffffff; // All pins of port A configured for very high speed! Page 201 of RM0451

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

	// Configure PA15 for altenate function (TIM2_CH1, pin 25 in LQFP32 package)
	GPIOA->OSPEEDR  |= BIT30; // MEDIUM SPEED
	GPIOA->OTYPER   &= ~BIT15; // Push-pull
	GPIOA->MODER    = (GPIOA->MODER & ~(BIT30)) | BIT31; // AF-Mode
	GPIOA->AFR[1]   |= BIT30 | BIT28 ; // AF5 selected (check table 16 in page 43 of "en.DM00108219.pdf")
	
	// Set up timer 2
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = SYSCLK/SPKR_F-1;
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	// Enable PWM in channel 1 of Timer 2
	TIM2->CCMR1|=BIT6|BIT5; // PWM mode 1 ([6..4]=110)
	TIM2->CCMR1|=BIT3; // OC1PE=1
	TIM2->CCER|=BIT0; // Bit 0 CC1E: Capture/Compare 1 output enable.
	
	// Set PWM to 50%
	TIM2->CCR1=(SYSCLK/(SPKR_F*2));
	TIM2->EGR |= BIT0; // UG=1
    TIM2->CR1 &= ~BIT0; // disable timer

/* 	// Set up timer 2
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = F_CPU/DEF_F-1;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting  */   
	
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

// A define to easily read PA14 (PA14 must be configured as input first)
#define PA14 (GPIOA->IDR & BIT14)

int main(void)
{
    int L, R;

	waitms(500); // Give putty a chance to start before we send characters with printf()
	eputs("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
	eputs("\r\nSTM32L051 multi I/O example.\r\n");
	eputs("Measures the voltage from ADC channels 8 and 9 (pins 14 and 15 of LQFP32 package)\r\n");
	eputs("Measures period on PA8 (pin 18 of LQFP32 package)\r\n");
	eputs("Toggles PB3, PB4, PB5, PB6, PB7 (pins 26, 27, 28, 29, 30 of LQFP32 package)\r\n");
	eputs("Generates servo PWMs on PA11, PA12 (pins 21, 22 of LQFP32 package)\r\n");
	eputs("Reads the push-button on pin PA14 (pin 24 of LQFP32 package)\r\n\r\n");

	Hardware_Init(); // configure pins, adc, lcd

	PB3_1;
	PB4_1;
	PB5_1;
	PB6_1;
	PB7_0; // unused for now
    
    // print initial
    LCDprint("Manual", 1, 1);
    LCDprint("follow: Short",2,1);
					
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
        if (togglethresh) {
            togglethresh = 0;
            if (lthresh == LTHRESH1) {
                lthresh = LTHRESH2;
                rthresh = RTHRESH2;
                LCDprint("Follow: Long",2,1);
            } else {
                lthresh = LTHRESH1;
                rthresh = RTHRESH1;
                LCDprint("Follow: Short",2,1);
            }
        }

        if (mode == 0)
        {
            L = readADC(ADC_CHSELR_CHSEL8);
            R = readADC(ADC_CHSELR_CHSEL9);
            printf("L: %d R: %d \r\n", L, R);
            if (R > rthresh)
            { // move R back
                PB6_1;
                PB5_0;
            }
            else if (R < rthresh - 150)
            { // move R forward
                PB6_0;
                PB5_1;
            }
            else
            {
                PB6_1;
                PB5_1;
                PB4_1;
                PB3_1;
            }

            if (L > lthresh)
            { // move L back
                PB4_1;
                PB3_0;
            }
            else if (L < lthresh - 150)
            { // move L forward
                PB4_0;
                PB3_1;
            }
            else
            {
                PB6_1;
                PB5_1;
                PB4_1;
                PB3_1;
            }
        }
        else
        {
        }

        if (toggle == 1)
        {
            toggle = 0;
            togglemode();
        }

        if (PA14)
        {
        }
        else
        {
            toggle = 0;
            togglemode();
            waitms(50);
        }

        waitms(50);
    }
}
