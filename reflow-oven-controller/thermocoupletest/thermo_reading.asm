$NOLIST
$MODLP51RC2
$LIST

CE_ADC    		EQU  P2.0
MY_MOSI_ADC   	EQU  P2.1 
MY_MISO_ADC   	EQU  P2.2
MY_SCLK_ADC   	EQU  P1.7

org 0000H
   ljmp MyProgram
   
CLK  EQU 22118400
BAUD equ 115200
BRG_VAL equ (0x100-(CLK/(16*BAUD)))

; These register definitions needed by 'math32.inc'
DSEG at 30H
x:   ds 4
y:   ds 4
bcd: ds 5
Result_Cold: ds 2
Result_Hot: ds 2

BSEG
mf: dbit 1

$NOLIST
$include(math32.inc)
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
$NOLIST
$include(LCD_4bit.inc)
$LIST

CSEG

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

; Send a character using the serial port
putchar:
    jnb TI, putchar
    clr TI
    mov SBUF, a
    ret

; Send a constant-zero-terminated string using the serial port
SendString:
    clr A
    movc A, @A+DPTR
    jz SendStringDone
    lcall putchar
    inc DPTR
    sjmp SendString
SendStringDone:
    ret

Send_BCD mac
	push ar0
	mov r0, %0
	lcall ?Send_BCD
	pop ar0
endmac

?Send_BCD:
	push acc
	mov a, r0
	swap a
	anl a, #0fh
	orl a, #30h
	lcall putchar
	mov a, r0
	anl a, #0fh
	orl a, #30h
	lcall putchar
	pop acc
	ret
	
Left_blank mac
	mov a, %0
	anl a, #0xf0
	swap a
	jz Left_blank_%M_a
	ljmp %1
Left_blank_%M_a:
	Display_char(#' ')
	mov a, %0
	anl a, #0x0f
	jz Left_blank_%M_b
	ljmp %1
Left_blank_%M_b:
	Display_char(#' ')
endmac

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

; We can display a number any way we want.  In this case with
; four decimal places.

wait_for_P4_5:
	jb P4.5, $ ; loop while the button is not pressed
	Wait_Milli_Seconds(#50) ; debounce time
	jb P4.5, wait_for_P4_5 ; it was a bounce, try again
	jnb P4.5, $ ; loop while the button is pressed
	ret

Test_msg:  db 'Temperature Test', 0

INI_SPI:
	setb MY_MISO_ADC ; Make MISO an input pin
	clr MY_SCLK_ADC           ; Mode 0,0 default
	ret
DO_SPI_G:
	mov R1, #0 ; Received byte stored in R1
	mov R2, #8            ; Loop counter (8-bits)
DO_SPI_G_LOOP:
	mov a, R0             ; Byte to write is in R0
	rlc a                 ; Carry flag has bit to write
	mov R0, a
	mov MY_MOSI_ADC, c
	setb MY_SCLK_ADC          ; Transmit
	mov c, MY_MISO_ADC        ; Read received bit
	mov a, R1             ; Save received bit in R1
	rlc a
	mov R1, a
	clr MY_SCLK_ADC
	djnz R2, DO_SPI_G_LOOP
	ret

delay_onesec:
	mov r2, #8
_delay_onesec:
	Wait_Milli_Seconds(#100)
	djnz r2, _delay_onesec
	ret
	

MyProgram:
	mov sp, #07FH ; Initialize the stack pointer
	; Configure P0 in bidirectional mode
    mov P0M0, #0
    mov P0M1, #0
    lcall LCD_4BIT
	Set_Cursor(1, 1)
    Send_Constant_String(#Test_msg)
    
    lcall INI_SPI
    lcall InitSerialPort

Forever:
	;=========T-Cold Manipulation and Calculation
	clr CE_ADC
	mov R0, #00000001B ; Start bit:1
	lcall DO_SPI_G
	mov R0, #10000000B ; Single ended, read channel 0
	lcall DO_SPI_G
	mov a, R1          ; R1 contains bits 8 and 9
	anl a, #00000011B  ; We need only the two least significant bits
	mov Result_Cold+1, a    ; Save result high.
	mov R0, #55H ; It doesn't matter what we transmit...
	lcall DO_SPI_G
	mov Result_Cold, R1     ; R1 contains bits 0 to 7.  Save result low.
	setb CE_ADC
	Wait_Milli_Seconds(#100)
	;NO CALCULATION REQUIRED Will be Added to hot juction calculations
	
	mov x+0, Result_Cold+0
	mov x+1, Result_Cold+1
	mov x+2, #0
	mov x+3, #0
	
	load_Y(410)
	lcall mul32
	
	load_Y(1023)
	lcall div32
	
	load_Y(273)
	lcall sub32
	
	mov Result_Cold+0, x+0
	mov Result_Cold+1, x+1

    lcall hex2bcd
    lcall Send_3_Digit_BCD

	;=============ADC Thermocouple Manipulation and Calculation
	clr CE_ADC
	mov R0, #00000001B ; Start bit:1
	lcall DO_SPI_G
	mov R0, #10010000B ; Single ended, read channel 0
	lcall DO_SPI_G
	mov a, R1          ; R1 contains bits 8 and 9
	anl a, #00000011B  ; We need only the two least significant bits
	mov Result_Hot+1, a    ; Save result high.
	mov R0, #55H ; It doesn't matter what we transmit...
	lcall DO_SPI_G
	mov Result_Hot, R1     ; R1 contains bits 0 to 7.  Save result low.
	setb CE_ADC
	Wait_Milli_Seconds(#100)
	
	mov x+0, Result_Hot+0
	mov x+1, Result_Hot+1
	mov x+2, #0
	mov x+3, #0

    lcall hex2bcd
    lcall Send_3_Digit_BCD

	mov y+0, #0
	mov y+1, #0
	mov y+2, #0
	mov y+3, #0	
	
	;load_Y(4096) ; takes ADC * 4.096 in essence
	;lcall mul32
	;load_Y(1000)
	;lcall div32
	;load_Y(1023)
	;lcall div32
	;load_Y(96)
	;lcall div32	
	;load_Y(24390) ; 
	;lcall mul32
		
	mov y+0, Result_Cold+0
	mov y+1, Result_Cold+1
	mov y+2, #0
	mov y+3, #0
	lcall add32

;add Hot and Cold junction together
	;mov x+0, Result_Hot+0
	;mov x+1, Result_Hot+1
	;mov x+2, Result_Hot+2
	;mov x+3, #0

	
	Wait_Milli_Seconds(#100)
	lcall hex2bcd
	lcall Display_10_digit_BCD
    lcall Send_3_Digit_BCD
    
    lcall delay_onesec
	ljmp Forever

Send_3_Digit_BCD: ;send 3 digits bcd in BCD var to putty
	Send_BCD(bcd+4)
	Send_BCD(bcd+3)
	Send_BCD(bcd+2)
    Send_BCD(bcd+1)
	Send_BCD(bcd+0)
	mov a, #'\r'
	lcall putchar
	mov a, #'\n'
	lcall putchar
ret
	
END
