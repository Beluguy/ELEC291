$NOLIST
$MODLP51RC2
$LIST

SHIFT_BUTTON equ P4.5 ;all push button positions are variable up to us
TEMP_SOAK_PB equ P4.6
TIME_SOAK_PB equ P4.7
TEMP_REFL_PB equ P4.8
TIME_REFL_PB equ P4.9

; Reset vector
org 0x0000
    ljmp main


; In the 8051 we can define direct access variables starting at location 0x30 up to location 0x7F
dseg at 0x30
temp_soak: ds 1
Time_soak: ds 1
Temp_refl: ds 1
Time_refl: ds 1

Change_8bit_Variable MAC
jb %0, %2
Wait_Milli_Seconds(#50) ; de-bounce
jb %0, %2
jnb %0, $
jb SHIFT_BUTTON, skip%Mb
dec %1
sjmp skip%Ma
skip%Mb:
inc %1
skip%Ma:
ENDMAC

Load_Configuration:
mov dptr, #0x7f84 ; First key value location.
getbyte(R0) ; 0x7f84 should contain 0x55
cjne R0, #0x55, Load_Defaults
getbyte(R0) ; 0x7f85 should contain 0xAA
cjne R0, #0xAA, Load_Defaults
; Keys are good. Get stored values.
mov dptr, #0x7f80
getbyte(temp_soak) ; 0x7f80
getbyte(time_soak) ; 0x7f81
getbyte(temp_refl) ; 0x7f82
getbyte(time_refl) ; 0x7f83
ret


Load_Defaults:
mov temp_soak, #150
mov time_soak, #45
mov temp_refl, #225
mov time_refl, #30
ret



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

SendToLCD: ;check slides
mov b, #100
div ab
orl a, #0x30 ; Convert hundreds to ASCII
lcall ?WriteData ; Send to LCD
mov a, b ; Remainder is in register b
mov b, #10
div ab
orl a, #0x30 ; Convert tens to ASCII
lcall ?WriteData; Send to LCD
mov a, b
orl a, #0x30 ; Convert units to ASCII
lcall ?WriteData; Send to LCD

ret

Initial_Message: db 'TS ts TR tR', 0
	
;---------------------------------;
; Main program. Includes hardware ;
; initialization and 'forever'    ;
; loop.                           ;
;---------------------------------;
main:
	; Initialization
    mov SP, #0x7F
   
    mov P0M0,#0
    mov P0M1,#0
    lcall Load_Configuration

    lcall LCD_4bit

    Set_Cursor(1,1)
    Send_Constant_String(#Initial_Message)
    ;display variables
    Set_Cursor(2,1)
    mov a, #42
    lcall SendToLCD
    Set_Cursor(2,5)
    mov a,temp_soak
    lcall SendToLCD
    Set_Cursor(2,9)
    mov a, Temp_refl
    lcall SendToLCD
    Set_Cursor(2,13)
    mov a, Time_refl
    lcall SendToLCD



	; After initialization the program stays in this 'forever' loop
loop:
  	
    Change_8bit_Variable(TEMP_SOAK_PB, temp_soak, loop_a)
    Set_Cursor(2, 1)
    mov a, temp_soak
    lcall SendToLCD
    lcall Save_Configuration
    loop_a:
    Change_8bit_Variable(TIME_SOAK_PB, time_soak, loop_a) ; check var names may have wrong caps
    Set_Cursor(2, 5)
    mov a, time_soak
    lcall SendToLCD
    lcall Save_Configuration
    loop_b:
    Change_8bit_Variable(TEMP_REFL_PB, temp_refl, loop_a)
    Set_Cursor(2, 1)
    mov a, temp_refl
    lcall SendToLCD
    lcall Save_Configuration
    loop_c:
    Change_8bit_Variable(TIME_REFL_PB, time_refl, loop_a)
    Set_Cursor(2, 1)
    mov a, time_refl
    lcall SendToLCD
    lcall Save_Configuration
    loop_d:

    ;/==========================FSM=======================\


    ;/======================COLD_JUNCTION HOT_JUNCTION========================\

    ljmp loop
END