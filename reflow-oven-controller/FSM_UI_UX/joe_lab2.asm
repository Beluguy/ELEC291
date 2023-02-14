; ISR_example.asm: a) Increments/decrements a BCD variable every half second using
; an ISR for timer 2; b) Generates a 2kHz square wave at pin P1.1 using
; an ISR for timer 0; and c) in the 'main' loop it displays the variable
; incremented/decremented using the ISR for timer 2 on the LCD.  Also resets it to 
; zero if the 'BOOT' pushbutton connected to P4.5 is pressed.
$NOLIST
$MODLP51RC2
$LIST

CLK           EQU 22118400 ; Microcontroller system crystal frequency in Hz
TIMER0_RATE   EQU 4096     ; 2048Hz squarewave (peak amplitude of CEM-1203 speaker)
TIMER0_RELOAD EQU ((65536-(CLK/TIMER0_RATE)))
TIMER0_RELOAD2 EQU ((65536-(CLK/3760)))
TIMER2_RATE   EQU 1000     ; 1000Hz, for a timer tick of 1ms
TIMER2_RELOAD EQU ((65536-(CLK/TIMER2_RATE)))

SOUND_OUT     equ P1.1
BUTTON4       equ P0.0
BUTTON3		  equ P0.3
BUTTON2		  equ P0.6				  
BUTTON1       equ P4.5
BUTTON0       equ P2.6

; Reset vector
org 0x0000
    ljmp main

; External interrupt 0 vector (not used in this code)
org 0x0003
	reti

; Timer/Counter 0 overflow interrupt vector
org 0x000B
	ljmp Timer0_ISR

; External interrupt 1 vector (not used in this code)
org 0x0013
	reti

; Timer/Counter 1 overflow interrupt vector (not used in this code)
org 0x001B
	reti

; Serial port receive/transmit interrupt vector (not used in this code)
org 0x0023 
	reti
	
; Timer/Counter 2 overflow interrupt vector
org 0x002B
	ljmp Timer2_ISR

; In the 8051 we can define direct access variables starting at location 0x30 up to location 0x7F
dseg at 0x30
Count1ms:       ds 2 ; Used to determine when half second has passed
BCD_counter:    ds 1 ; The BCD counter incrememted in the ISR and displayed in the main loop
Minutes_Counter:ds 1 ;
Hours_Counter:  ds 1 ;

aMinutes_Counter: ds 1 ; alarm
aHours_Counter:   ds 1 ; alarm

edit_segment:	ds 1 ; which segment are we editing 0 - hours; 1 - minutes; 2 - seconds; 3 - ampm; 4 - ahours; 5 - aminutes; 6 - a ampm

; In the 8051 we have variables that are 1-bit in size.  We can use the setb, clr, jb, and jnb
; instructions with these variables.  This is how you define a 1-bit variable:
bseg
one_seconds_flag: dbit 1 ; Set to one in the ISR every time 1000 ms had passed
AM_PM: 			  dbit 1 ; Set according to am or pm
alarm: 			  dbit 1 ; alarm on or off
aAM_PM:			  dbit 1 ; alarm am/pm
tone:			  dbit 1 ; tone flag
edit:			  dbit 1 ; edit flag

cseg
; These 'equ' must match the hardware wiring
LCD_RS equ P3.2
;LCD_RW equ PX.X ; Not used in this code, connect the pin to GND
LCD_E  equ P3.3
LCD_D4 equ P3.4
LCD_D5 equ P3.5
LCD_D6 equ P3.6
LCD_D7 equ P3.7

$NOLIST
$include(LCD_4bit.inc) ; A library of LCD related functions and utility macros
$LIST

;                     1234567890123456    <- This helps determine the location of the counter
Initial_Message:  db 'Time  xx:xx:xxX ', 0
Initial_Message2: db 'Alarm xx:xxX    ', 0
On_Message: 	  db 'on ', 0
Off_Message:      db 'off', 0

;---------------------------------;
; Routine to initialize the ISR   ;
; for timer 0                     ;
;---------------------------------;
Timer0_Init:
	mov a, TMOD
	anl a, #0xf0 ; 11110000 Clear the bits for timer 0
	orl a, #0x01 ; 00000001 Configure timer 0 as 16-timer
	mov TMOD, a
	mov TH0, #high(TIMER0_RELOAD)
	mov TL0, #low(TIMER0_RELOAD)
	; Set autoreload value
	mov RH0, #high(TIMER0_RELOAD)
	mov RL0, #low(TIMER0_RELOAD)
	; Enable the timer and interrupts
    setb ET0  ; Enable timer 0 interrupt
    ; setb TR0  ; Start timer 0
	ret

;---------------------------------;
; ISR for timer 0.  Set to execute;
; every 1/4096Hz to generate a    ;
; 2048 Hz square wave at pin P1.1 ;
;---------------------------------;
Timer0_ISR:
	;clr TF0  ; According to the data sheet this is done for us already.
	cpl SOUND_OUT ; Connect speaker to P1.1!
	jb tone, annoying
	mov TH0, #high(TIMER0_RELOAD)
	mov TL0, #low(TIMER0_RELOAD)
	; Set autoreload value
	mov RH0, #high(TIMER0_RELOAD)
	mov RL0, #low(TIMER0_RELOAD)
	reti
	
annoying:
	mov TH0, #high(TIMER0_RELOAD2)
	mov TL0, #low(TIMER0_RELOAD2)
	; Set autoreload value
	mov RH0, #high(TIMER0_RELOAD2)
	mov RL0, #low(TIMER0_RELOAD2)
	reti

;---------------------------------;
; Routine to initialize the ISR   ;
; for timer 2                     ;
;---------------------------------;
Timer2_Init:
	mov T2CON, #0 ; Stop timer/counter.  Autoreload mode.
	mov TH2, #high(TIMER2_RELOAD)
	mov TL2, #low(TIMER2_RELOAD)
	; Set the reload value
	mov RCAP2H, #high(TIMER2_RELOAD)
	mov RCAP2L, #low(TIMER2_RELOAD)
	; Init One millisecond interrupt counter.  It is a 16-bit variable made with two 8-bit parts
	clr a
	mov Count1ms+0, a
	mov Count1ms+1, a
	; Enable the timer and interrupts
    setb ET2  ; Enable timer 2 interrupt
    setb TR2  ; Enable timer 2
	ret

;---------------------------------;
; ISR for timer 2                 ;
;---------------------------------;
Timer2_ISR:
	clr TF2  ; Timer 2 doesn't clear TF2 automatically. Do it in ISR
	cpl P1.0 ; To check the interrupt rate with oscilloscope. It must be precisely a 1 ms pulse.
	
	; The two registers used in the ISR must be saved in the stack
	push acc
	push psw
	
	; Increment the 16-bit one mili second counter
	inc Count1ms+0    ; Increment the low 8-bits first
	mov a, Count1ms+0 ; If the low 8-bits overflow, then increment high 8-bits
	jnz Inc_Done
	inc Count1ms+1

Inc_Done:
	; Check if second has passed
	mov a, Count1ms+0
	cjne a, #low(1000), Timer2_ISR_done ; Warning: this instruction changes the carry flag!
	mov a, Count1ms+1
	cjne a, #high(1000), Timer2_ISR_done
	
	; 1000 milliseconds have passed.  Set a flag so the main program knows
	setb one_seconds_flag ; Let the main program know second had passed
	
	cpl tone 
	
	; Reset to zero the milli-seconds counter, it is a 16-bit variable
	clr a
	mov Count1ms+0, a
	mov Count1ms+1, a
	
	; reset BCD_counter if hits 60, increment 1 to minutes
	; Increment the seconds counter
	mov a, BCD_counter
	cjne a, #0x59, Timer2_ISR_increment_s
	clr a
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov BCD_counter, a
	
	; increment the minutes counter
	mov a, Minutes_Counter
	cjne a, #0x59, Timer2_ISR_increment_m
	clr a
	da a
	mov Minutes_Counter, a
	
	; increment the hours counter
	mov a, Hours_Counter
	cjne a, #0x11, Timer2_ISR_increment_h
	clr a
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov Hours_Counter, a
	
	; toggle AM/PM
	cpl AM_PM
	
	ljmp Timer2_ISR_done
	
Timer2_ISR_increment_s:
	add a, #0x01
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov BCD_counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_decrement_s:
	add a, #0x99 ; Adding the 10-complement of -1 is like subtracting 1.
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov BCD_counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_increment_m:
	add a, #0x01
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov Minutes_Counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_decrement_m:
	add a, #0x99 ; Adding the 10-complement of -1 is like subtracting 1.
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov Minutes_Counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_increment_h:
	add a, #0x01
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov Hours_Counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_decrement_h:
	add a, #0x99 ; Adding the 10-complement of -1 is like subtracting 1.
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov Hours_Counter, a
	ljmp Timer2_ISR_done
Timer2_ISR_done:
	pop psw
	pop acc
	reti

;---------------------------------;
; Main program. Includes hardware ;
; initialization and 'forever'    ;
; loop.                           ;
;---------------------------------;
main:
	; Initialization
    mov SP, #0x7F
    lcall Timer0_Init
    lcall Timer2_Init
    ; In case you decide to use the pins of P0, configure the port in bidirectional mode:
    mov P0M0, #0
    mov P0M1, #0
    setb EA   ; Enable Global interrupts
    lcall LCD_4BIT
    ; For convenience a few handy macros are included in 'LCD_4bit.inc':
	Set_Cursor(1, 1)
    Send_Constant_String(#Initial_Message)
    Set_Cursor(2, 1)
    Send_Constant_String(#Initial_Message2)
    setb one_seconds_flag
	
	; SET INITIAL VALUES
	mov BCD_counter, #0x00
	mov Minutes_Counter, #0x00
	mov Hours_Counter, #0x00
	mov aMinutes_Counter, #0x00
	mov aHours_Counter, #0x00
	clr AM_PM
	clr aAM_PM
	clr alarm
	clr edit
	
	mov edit_segment, #0
	
	; After initialization the program stays in this 'forever' loop
loop:
	jb BUTTON0, button_1  ; if the 'BOOT' button is not pressed skip
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb BUTTON0, button_1  ; if the 'BOOT' button is not pressed skip
	jnb BUTTON0, $		; Wait for button release.  The '$' means: jump to same instruction.
	; A valid press of the 'BOOT' button has been detected
	cpl alarm
	ljmp loop_b             ; Display the new value
button_1:
	jb BUTTON1, button_2  ; if the 'BOOT' button is not pressed skip
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb BUTTON1, button_2  ; if the 'BOOT' button is not pressed skip
	jnb BUTTON1, $		; Wait for button release.  The '$' means: jump to same instruction.
	; A valid press of the 'BOOT' button has been detected
	cpl TR2
	cpl edit
	cpl P2.0 ; LED on to signify edit mode
button_2:
	jb BUTTON2, button_3  ; if the 'BOOT' button is not pressed skip
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb BUTTON2, button_3  ; if the 'BOOT' button is not pressed skip
	jnb BUTTON2, $		; Wait for button release.  The '$' means: jump to same instruction.
	; A valid press of the 'BOOT' button has been detected
	jnb edit, button_3
	mov a, edit_segment
	cjne a, #6, edit_segment_add
	mov a, #0
	mov edit_segment, a
	sjmp button_3
edit_segment_add:
	add a, #1
	mov edit_segment, a
button_3: ; increment 0 - hours; 1 - minutes; 2 - seconds; 3 - ampm; 4 - ahours; 5 - aminutes; 6 - a ampm
	jb BUTTON3, button_4  ; if the 'BOOT' button is not pressed skip
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb BUTTON3, button_4  ; if the 'BOOT' button is not pressed skip
	jnb BUTTON3, $		; Wait for button release.  The '$' means: jump to same instruction.
	; A valid press of the 'BOOT' button has been detected
	jnb edit, button_4
	mov a, edit_segment
	; six_i
	cjne a, #6, five_i
	cpl aAM_PM
	ljmp loop_b             ; Display the new value
five_i:
	cjne a, #5, four_i
	mov a, aMinutes_Counter
	add a, #0x01
	da a
	mov aMinutes_Counter,a
	ljmp loop_b             ; Display the new value
four_i:
	cjne a, #4, three_i
	mov a, aHours_Counter
	add a, #0x01
	da a
	mov aHours_Counter, a
	ljmp loop_b             ; Display the new value
three_i:
	cjne a, #3, two_i
	cpl AM_PM
	ljmp loop_b             ; Display the new value
two_i:
	cjne a, #2, one_i
	mov a, BCD_counter
	add a, #0x01
	da a
	mov BCD_counter, a
	ljmp loop_b
one_i:
	cjne a, #1, zero_i
	mov a, Minutes_Counter
	add a, #0x01
	da a
	mov Minutes_Counter, a
	ljmp loop_b             ; Display the new value
zero_i:
	cjne a, #0, button_4
	mov a, Hours_Counter
	add a, #0x01
	da a
	mov Hours_Counter, a
	ljmp loop_b             ; Display the new value
button_4: ; decrement 0 - hours; 1 - minutes; 2 - seconds; 3 - ampm; 4 - ahours; 5 - aminutes; 6 - a ampm
	jb BUTTON4, loop_a  ; if the 'BOOT' button is not pressed skip
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb BUTTON4, loop_a  ; if the 'BOOT' button is not pressed skip
	jnb BUTTON4, $		; Wait for button release.  The '$' means: jump to same instruction.
	; A valid press of the 'BOOT' button has been detected
	jnb edit, loop_b
	mov a, edit_segment
	; six_i
	cjne a, #6, five_d
	cpl aAM_PM
	ljmp loop_b             ; Display the new value
five_d:
	cjne a, #5, four_d
	mov a, aMinutes_Counter
	add a, #0x99
	da a
	mov aMinutes_Counter,a
	ljmp loop_b             ; Display the new value
four_d:
	cjne a, #4, three_d
	mov a, aHours_Counter
	add a, #0x99
	da a
	mov aHours_Counter, a
	ljmp loop_b             ; Display the new value
three_d:
	cjne a, #3, two_d
	cpl AM_PM
	ljmp loop_b             ; Display the new value
two_d:
	cjne a, #2, one_d
	mov a, BCD_counter
	add a, #0x99
	da a
	mov BCD_counter, a
	ljmp loop_b
one_d:
	cjne a, #1, zero_d
	mov a, Minutes_Counter
	add a, #0x99
	da a
	mov Minutes_Counter, a
	ljmp loop_b             ; Display the new value
zero_d:
	cjne a, #0, loop_a
	mov a, Hours_Counter
	add a, #0x99
	da a
	mov Hours_Counter, a
	ljmp loop_b             ; Display the new value
loop_a:
	jb one_seconds_flag, loop_b
	ljmp loop
loop_b: ; loops every one second
    clr one_seconds_flag ; We clear this flag in the main loop, but it is set in the ISR for timer 2
    
    ; main clock    
	Set_Cursor(1, 7)
	Display_BCD(Hours_Counter)
	Set_Cursor(1, 10)
	Display_BCD(Minutes_Counter)
	Set_Cursor(1, 13)     ; the place in the LCD where we want the BCD counter value
	Display_BCD(BCD_counter) ; This macro is also in 'LCD_4bit.inc'
	
	; alarm display
	Set_Cursor(2, 7)
	Display_BCD(aHours_Counter)
	Set_Cursor(2, 10)
	Display_BCD(aMinutes_Counter)
	
	; alarm am/pm
	Set_Cursor(2, 12)
	jnb aAM_PM, ifaAM
	Display_char(#'P')
loop_d:
	; for alarm buzzer
	jnb alarm, alarm_off
	
	mov a, Minutes_Counter
	cjne a, aMinutes_Counter, alarm_off

	mov a, Hours_Counter
	cjne a, aHours_Counter, alarm_off

	mov a, AM_PM
	cjne a, aAM_PM, alarm_off
	setb TR0 ; alarm on, enable timer0
	
loop_e:
	; for alarm enable display
	Set_Cursor(2, 14)
	jb alarm, ifOn
	Send_Constant_String(#Off_Message)
loop_c:
	; for main clock am/pm display
	Set_Cursor(1, 15)
	jnb AM_PM, ifAM
	Display_char(#'P')
    ljmp loop
    

alarm_off:
	clr TR0
	ljmp loop_e
ifOn:
	Send_Constant_String(#On_Message)
	ljmp loop_c
ifaAM:
	Display_char(#'A')
	ljmp loop_d
ifAM:
	Display_char(#'A')
	ljmp loop
END
