cseg

; When using a 22.1184MHz crystal in fast mode
; one cycle takes 1.0/22.1184MHz = 45.21123 ns

;---------------------------------;
; Wait 40 microseconds            ;
;---------------------------------;
Wait40uSec:
	push AR0
	mov R0, #177
L0: 
	nop
	nop
	djnz R0, L0 ; 1+1+3 cycles->5*45.21123ns*177=40us
	pop AR0
    ret

;---------------------------------;
; Wait 'R2' milliseconds          ;
;---------------------------------;
Wait_Milli_Seconds mac
	push AR2
	mov R2, %0
	lcall ?Wait_Milli_Seconds
	pop AR2
endmac

?Wait_Milli_Seconds:
	push AR0
	push AR1
L3: mov R1, #45
L2: mov R0, #166
L1: djnz R0, L1 ; 3 cycles->3*45.21123ns*166=22.51519us
    djnz R1, L2 ; 22.51519us*45=1.013ms
    djnz R2, L3 ; number of millisecons to wait passed in R2
    pop AR1
    pop AR0
    ret
	
;---------------------------------;
; Toggles the 'E' pin in the LCD  ;
;---------------------------------;
LCD_pulse:
	setb LCD_E
	lcall Wait40uSec
	clr LCD_E
    ret

;---------------------------------;
; Writes acc to LCD in 4-bit mode ;
;---------------------------------;
LCD_byte:
	; Write high 4 bits first
	mov c, ACC.7
	mov LCD_D7, c
	mov c, ACC.6
	mov LCD_D6, c
	mov c, ACC.5
	mov LCD_D5, c
	mov c, ACC.4
	mov LCD_D4, c
    lcall LCD_pulse
    lcall Wait40uSec
	; Write low 4 bits next
	mov c, ACC.3
	mov LCD_D7, c
	mov c, ACC.2
	mov LCD_D6, c
	mov c, ACC.1
	mov LCD_D5, c
	mov c, ACC.0
	mov LCD_D4, c
    lcall LCD_pulse
	ret

;---------------------------------;
; Write data to LCD               ;
;---------------------------------;
WriteData mac
	mov a, %0
	lcall ?WriteData
endmac
	
?WriteData:
	setb LCD_RS
	ljmp LCD_byte

;---------------------------------;
; Write command to LCD            ;
;---------------------------------;
WriteCommand mac
	mov a, %0
	lcall ?WriteCommand
endmac

?WriteCommand:
	clr LCD_RS
	ljmp LCD_byte

;---------------------------------;
; Configure LCD in 4-bit mode     ;
;---------------------------------;
LCD_4BIT:
	clr LCD_E   ; Resting state of LCD's enable pin is zero
	; clr LCD_RW  ; Only writing to the LCD.  Tie this pin to ground
	
	; After power on, let the LCD start up before initializing
	; NOTE: the preprogrammed power-on delay of 16 ms on the AT89LP51RC2
	; seems to be enough.  That is why this line is commented out.
	; Wait_Milli_Seconds(#40)
	
	; First make sure the LCD is in 8-bit mode and then change to 4-bit mode
	WriteCommand(#0x33)
	WriteCommand(#0x33)
	WriteCommand(#0x32) ; change to 4-bit mode

	; Configure the LCD
	WriteCommand(#0x28)
	WriteCommand(#0x0c)
	WriteCommand(#0x01) ;  Clear screen command (takes some time)

    ;Wait for the clear screen command to finish.
	Wait_Milli_Seconds(#2)
    ret

;---------------------------------;
; Send a constant string to LCD   ;
;---------------------------------;
Send_Constant_String mac
	push dph
	push dpl
	push acc
	mov dptr, %0
	lcall ?Send_Constant_String
	pop acc
	pop dpl
	pop dph
endmac

?Send_Constant_String:
    clr a
    movc a, @a+dptr
    jz ?Send_Constant_String_Done
    lcall ?WriteData
    inc dptr
    sjmp ?Send_Constant_String
?Send_Constant_String_Done:
    ret	

;---------------------------------;
; Set LCD cursor at row, column   ;
;---------------------------------;
Set_Cursor mac
	push acc
	mov a, #%1
	dec a
	lcall ?Set_Cursor_%0 ; Select column and row
	pop acc
endmac

?Set_Cursor_2:
	orl a, #01000000B
?Set_Cursor_1:
	orl a, #10000000B
	ljmp ?WriteCommand ; Select column and row

;---------------------------------;
; Display a BCD number in the LCD ;
;---------------------------------;
Display_BCD mac
	push ar0
	mov r0, %0
	lcall ?Display_BCD
	pop ar0
endmac

?Display_BCD:
	push acc
	; Write most significant digit
	mov a, r0
	swap a
	anl a, #0fh
	orl a, #30h
	lcall ?WriteData
	; write least significant digit
	mov a, r0
	anl a, #0fh
	orl a, #30h
	lcall ?WriteData
	pop acc
	ret

;------------------------------------;
; Display a char in the LCD          ;
;------------------------------------;
Display_char mac
	push acc
	mov a, %0
	lcall ?WriteData
	pop acc
endmac

	