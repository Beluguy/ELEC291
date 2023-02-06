$MODLP51RC2
org 0000H
   ljmp MainProgram

; Timer/Counter 0 overflow interrupt vector
org 0x000B
	ljmp Timer0_ISR

CLK  EQU 22118400
BAUD equ 115200
BRG_VAL equ (0x100-(CLK/(16*BAUD)))
TIMER0_RATE   EQU 4096     ; 2048Hz squarewave (peak amplitude of CEM-1203 speaker)
TIMER0_RELOAD EQU ((65536-(CLK/TIMER0_RATE)))

; These register definitions needed by 'math32.inc'
DSEG at 30H
x:   ds 4
y:   ds 4
bcd: ds 5
Result: ds 2

BSEG
mf: dbit 1

CSEG

$NOLIST
$include(math32.inc)
$include(macros.inc)
$LIST

; These 'equ' must match the hardware wiring
; They are used by 'LCD_4bit.inc'
LCD_RS equ P3.2
; LCD_RW equ Px.x ; Always grounded
LCD_E  equ P3.3
LCD_D4 equ P3.4
LCD_D5 equ P3.5
LCD_D6 equ P3.6
LCD_D7 equ P3.7
; These ’EQU’ must match the wiring between the microcontroller and ADC 
CE_ADC EQU P2.0 
MY_MOSI EQU P2.1 
MY_MISO EQU P2.2 
MY_SCLK EQU P2.3 

SOUND_OUT     equ P1.1
$NOLIST
$include(LCD_4bit.inc)
$LIST

Initial_Message:  db 'Temperature (C):', 0

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
	mov TH0, #high(TIMER0_RELOAD)
	mov TL0, #low(TIMER0_RELOAD)
	; Set autoreload value
	mov RH0, #high(TIMER0_RELOAD)
	mov RL0, #low(TIMER0_RELOAD)
	reti

; Configure the serial port and baud rate
InitSerialPort:
    ; Since the reset button bounces, we need to wait a bit before
    ; sending messages, otherwise we risk displaying gibberish!
	mov R1, #222
    mov R0, #166
    djnz R0, $   ; 3 cycles->3*45.21123ns*166=22.51519us
    djnz R1, $-4 ; 22.51519us*222=4.998ms
    ; Now we can proceed with the configuration
	orl	PCON,#0x80
	mov	SCON,#0x52
	mov	BDRCON,#0x00
	mov	BRL,#BRG_VAL
	mov	BDRCON,#0x1E ; BDRCON=BRR|TBCK|RBCK|SPD;
    ret
    
INIT_SPI: 
	setb MY_MISO ; Make MISO an input pin
	clr MY_SCLK ; For mode (0,0) SCLK is zero
	ret

MainProgram: ; setup()
    mov SP, #7FH ; Set the stack pointer to the begining of idata
    
    lcall InitSerialPort
    lcall INIT_SPI
    lcall LCD_4BIT
    
    lcall Timer0_Init
    setb EA   ; Enable Global interrupts
    
    Set_Cursor(1, 1)
    Send_Constant_String(#Initial_Message)

Forever: ;loop() please only place function calls into the loop!
    lcall readADC ; reads ch0 and saves result to Result as 2 byte binary
	lcall Delay ; hardcoded 1s delay can change
	lcall Do_Something_With_Result ; convert to bcd and send to serial

	ljmp Forever

readADC:
    clr CE_ADC
	mov R0, #00000001B ; Start bit:1
	lcall DO_SPI_G
	mov R0, #10000000B ; Single ended, read channel 0
	lcall DO_SPI_G
	mov a, R1 ; R1 contains bits 8 and 9
	anl a, #00000011B ; We need only the two least significant bits
	mov Result+1, a ; Save result high.
	mov R0, #55H ; It doesn't matter what we transmit...
	lcall DO_SPI_G
	mov Result, R1 ; R1 contains bits 0 to 7. Save result low.
	setb CE_ADC
    ret

Do_Something_With_Result:
	mov x+0, result+0
	mov x+1, result+1
	mov x+2, #0
	mov x+3, #0
	
	load_Y(410)
	lcall mul32
	
	load_Y(1023)
	lcall div32
	
	load_Y(273)
	lcall sub32
	
	lcall hex2bcd
	lcall Display_10_digit_BCD
	
	lcall Send_10_digit_BCD
	
	mov a, x
	cjne a, #50, NOT_EQ
	NOT_EQ: JC REQ_LOW
	setb TR0
	ret
	REQ_LOW:
	clr TR0
	ret
	
Delay:
	mov R2, #200
    mov R1, #222
    mov R0, #166
    djnz R0, $   ; 3 cycles->3*45.21123ns*166=22.51519us
    djnz R1, $-4 ; 22.51519us*222=4.998ms
    djnz R2, $-4 ; 0.996 seconds
    ret

DO_SPI_G: 
	push acc 
	mov R1, #0 ; Received byte stored in R1
	mov R2, #8 ; Loop counter (8-bits)
DO_SPI_G_LOOP: 
	mov a, R0 ; Byte to write is in R0
	rlc a ; Carry flag has bit to write
	mov R0, a 
	mov MY_MOSI, c 
	setb MY_SCLK ; Transmit
	mov c, MY_MISO ; Read received bit
	mov a, R1 ; Save received bit in R1
	rlc a 
	mov R1, a 
	clr MY_SCLK 
	djnz R2, DO_SPI_G_LOOP 
 	pop acc 
 	ret

; Sends 10-digit BCD number in bcd to the LCD
Display_10_digit_BCD:
	Set_Cursor(2, 7)
	Display_BCD(bcd+4)
	Display_BCD(bcd+3)
	Display_BCD(bcd+2)
	Display_BCD(bcd+1)
	Display_BCD(bcd+0)
	; Replace all the zeros to the left with blanks
	Set_Cursor(2, 7)
	Left_blank(bcd+4, skip_blank)
	Left_blank(bcd+3, skip_blank)
	Left_blank(bcd+2, skip_blank)
	Left_blank(bcd+1, skip_blank)
	mov a, bcd+0
	anl a, #0f0h
	swap a
	jnz skip_blank
	Display_char(#' ')
skip_blank:
	ret
 	
Send_10_Digit_BCD:
	Send_BCD(bcd+0)
	mov a, #'\r'
	lcall putchar
	mov a, #'\n'
	lcall putchar
	ret

; Send a character using the serial port
putchar:
    jnb TI, putchar
    clr TI
    mov SBUF, a
    ret

END
