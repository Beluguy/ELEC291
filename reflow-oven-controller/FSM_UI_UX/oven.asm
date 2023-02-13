$MODLP51RC2
org 0000H
   ljmp MainProgram

; Timer/Counter 0 overflow interrupt vector
org 0x000B
	ljmp Timer0_ISR

; Timer/Counter 2 overflow interrupt vector
org 0x002B
	ljmp Timer2_ISR

CLK  				EQU 22118400
BAUD 				EQU 115200
BRG_VAL 			EQU (0x100-(CLK/(16*BAUD)))
TIMER0_RATE   		EQU 1000    ; 1000Hz PWM output signal 
TIMER0_RELOAD 		EQU ((65536-(CLK/TIMER0_RATE)))
TIMER2_RATE     	EQU 1000     ; 1000Hz, for a timer tick of 1ms
TIMER2_RELOAD   	EQU ((65536-(CLK/TIMER2_RATE)))

HOLD_PWM			EQU 20		; 20% pwm for holding the temp constant 
PWM_20				EQU (TIMER0_RATE-(HOLD_PWM*10))

; These register definitions needed by 'math32.inc'
DSEG at 30H
x:   				ds 4
y:   				ds 4
bcd: 				ds 5
Result: 			ds 2

;--------------------for clock----------------------
Count1ms:       	ds 2 ; Used to determine when one second has passed
secs_ctr:       	ds 1
mins_ctr:       	ds 1
pwm_time: 			ds 1 ; Used to check whether it is time to turn on the pwm output
;---------------------------------------------------

;--------------------for settings-------------------
edit_sett:	        ds 1 ; which segment are we editing 
; 0 - soak temp
; 1 - soak time
; 2 - reflow temp
; 3 - reflow time
; 4 - cool temp

;---------------------------------------------------

;--------------------for FSM------------------------
state: 				ds 1				
soak_temp: 			ds 1
soak_time: 			ds 1
reflow_temp: 		ds 1
reflow_time: 		ds 1
pwm: 				ds 1
sec: 				ds 1
cool_temp:			ds 1
temp:				ds 1
;---------------------------------------------------

BSEG
mf: 				dbit 1 ; flag for math32
start_flag: 		dbit 1
one_second_flag: 	dbit 1 ; Set to one in the ISR every time 1000 ms had passed
safety_overheat:    dbit 1 ; for overheat safety feature


CSEG

$NOLIST
$include(math32.inc)
$include(macros.inc)
$LIST

; These 'equ' must match the hardware wiring
; They are used by 'LCD_4bit.inc'
LCD_RS 			EQU P3.2
; LCD_RW equ Px.x ; Always grounded
LCD_E  			EQU P3.3
LCD_D4 			EQU P3.4
LCD_D5 			EQU P3.5
LCD_D6 			EQU P3.6
LCD_D7 			EQU P3.7
; These ’EQU’ must match the wiring between the microcontroller and ADC 
CE_ADC 			EQU P2.0 
MY_MOSI_ADC	EQU P2.1 
MY_MISO_ADC 	EQU P2.2 
MY_SCLK_ADC 	EQU P2.3 

SOUND_OUT     	EQU P1.1
RST				EQU	P4.5	; button to reset
EDIT			EQU P0.6	; button for changing what to edit
INCR            EQU P0.3   ; button to increment current selection
DECR            EQU P0.0   ; button to increment current selection
START_STOP 		EQU P2.4 	; button to start/stop reflow
OUTPUT			EQU P	; output signal to the relay box

; i have buttons on 2.4, 4.5, 0.6, 0.3, 0.0 (left to right)

$NOLIST
$include(LCD_4bit.inc)
$LIST

;------------------UI-UX vars---------------------
;            1234567890123456
setup1:  db 'soak            ', 0
setup2:  db 'tmp:XXX time:XXX', 0
setup3:  db 'refl            ', 0
setup4:  db 'cool *          ', 0
setup5:  db 'temp:XXX        ', 0

run1:    db 'temp:XXX state X', 0
run2:    db 'elapsed XX:XX   ', 0

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
	mov TH0, #high(TIMER0_RELOAD)
	mov TL0, #low(TIMER0_RELOAD)
	; Set autoreload value
	mov RH0, #high(TIMER0_RELOAD)
	mov RL0, #low(TIMER0_RELOAD)
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
	setb one_second_flag ; Let the main program know second had passed
		
	; Reset to zero the milli-seconds counter, it is a 16-bit variable
	clr a
	mov Count1ms+0, a
	mov Count1ms+1, a
	
	; reset secs_ctr if hits 60, increment 1 to minutes
	; Increment the seconds counter
	mov a, secs_ctr
	cjne a, #0x59, Timer2_ISR_increment_s
	clr a
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov secs_ctr, a
	
	; increment the minutes counter
	mov a, mins_ctr
	cjne a, #0x59, Timer2_ISR_increment_m
	clr a
	da a
	mov mins_ctr, a

	ljmp Timer2_ISR_done
	
Timer2_ISR_increment_s:
	add a, #0x01
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov secs_ctr, a
	ljmp Timer2_ISR_done
Timer2_ISR_increment_m:
	add a, #0x01
	da a ; Decimal adjust instruction.  Check datasheet for more details!
	mov mins_ctr, a
	ljmp Timer2_ISR_done
Timer2_ISR_done:
	pop psw
	pop acc
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
	setb MY_MISO_ADC ; Make MISO an input pin
	clr MY_SCLK_ADC ; For mode (0,0) SCLK is zero
	ret

; -------------------------------------------------- MAIN PROGRAM LOOP ----------------------------------------------

MainProgram: ; setup()
    mov SP, #7FH 						; Set the stack pointer to the begining of idata
    
	clr OUTPUT							; pwm is set to low by default
	lcall Load_Defaults
    lcall InitSerialPort
    lcall INIT_SPI
    lcall LCD_4BIT

    mov start_flag, #0
    mov safety_overheat, #0

    
    lcall Timer0_Init
    setb EA   							; Enable Global interrupts

forever: ;loop() please only place function calls into the loop!
    jnb one_second_flag, skipDisplay 	; this segment only executes once a second
    clr one_second_flag
    lcall generateDisplay
    lcall readADC 						; reads ch0 and saves result to Result as 2 byte binary
	;lcall Delay ; hardcoded 1s delay can change or use the Timer // COMMENTED SINCE WE ARE USING TIMER NOW
    lcall Do_Something_With_Result ; convert to bcd and send to serial
    lcall checkOverheat
    skipDisplay: 						; end segment

    jb start_flag, skipPoll
    lcall pollButtons 					; poll buttons for editing screen
    skipPoll: 

    lcall reset 						; check if reset is pressed
    ljmp FSM 							; finite state machine logic
	lcall save_config					; save config to nvmem
	ljmp forever

; ---------------------------------------------------------------------------------------------------

;----------------------------------safety-features---------------------------------------------------
checkOverheat:
    mov a, temp
	clr c
	subb a, #251				; if 251 > temp, c = 1
	jc notOverheat				; return if notOverheating
    jb safety_overheat, overheatReset ; check if flag is set, if set that means has been overheating for prolonged time
	setb safety_overheat        ; set overheat flag for next time
    ret
notOverheat:
    clr safety_overheat
	ret
overheatReset:
    clr safety_overheat
    mov a, #5						; reset to state 5 when reset for safety
    ret
;----------------------------------------------------------------------------------------------------

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
	lcall Send_3_digit_BCD
	
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
	mov MY_MOSI_ADC, c 
	setb MY_SCLK_ADC ; Transmit
	mov c, MY_MISO_ADC ; Read received bit
	mov a, R1 ; Save received bit in R1
	rlc a 
	mov R1, a 
	clr MY_SCLK_ADC 
	djnz R2, DO_SPI_G_LOOP 
 	pop acc 
 	ret
 	
Send_3_Digit_BCD: ;send 3 digits bcd in BCD var to putty
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    lcall putchar
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

;----------------------------------UI CODE----------------------------------------------
generateDisplay:
    jb start_flag, startDisplay
    ljmp setupDisplay

startDisplay:
    Set_Cursor(1,1)
    Send_Constant_String(#run1)
    Set_Cursor(2,1)
    Send_Constant_String(#run2)
    
    Set_Cursor(1,6)
    load_X(0)
    mov x+0, temp
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0) ;display digit 2 and 1

    Set_Cursor(1,16)
    load_X(0)
    mov x+0, state
    lcall hex2bcd
    ; Display digit 1
    mov a, bcd+0
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)

    Set_Cursor(2,9)
    Display_BCD(mins_ctr)
    Set_Cursor(2,12)
    Display_BCD(secs_ctr)
    ret

;             1234567890123456
;setup1:  db 'soak            ', 0
;setup2:  db 'tmp:XXX time:XXX', 0
;setup3:  db 'reflow          ', 0

;run1:    db 'temp:XXX state X', 0
;run2:    db 'elapsed XX:XX   ', 0

setupDisplay:
    mov a, edit_sett
    cjne a, #0, checkScreen2
    ljmp soakScreen
    cjne a, #1, checkScreen2
    ljmp soakScreen
checkScreen2:
    cjne a, #2, checkScreen3
    ljmp reflowScreen
    cjne a, #3, checkScreen3
    ljmp reflowScreen
checkScreen3:
    ljmp coolScreen
soakScreen:
    Set_Cursor(1,1)
    Send_Constant_String(#setup1)
    Set_Cursor(2,1)
    Send_Constant_String(#setup2)

    Set_Cursor(2,5)
    load_X(0)
    mov x+0, soak_temp
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0)

    Set_Cursor(2,14)
    load_X(0)
    mov x+0, soak_time
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0)

    mov a, edit_sett
    cjne a, #0, indic_soak_time
    Set_Cursor(1,6)
    sjmp indic_soak_next
indic_soak_time:
    Set_Cursor(1,15)
indic_soak_next:
    Display_char(#'*')
    ret
reflowScreen:
    Set_Cursor(1,1)
    Send_Constant_String(#setup3)
    Set_Cursor(2,1)
    Send_Constant_String(#setup2)
  
    Set_Cursor(2,5)
    load_X(0)
    mov x+0, reflow_temp
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0)
    Set_Cursor(2,14)
    load_X(0)
    mov x+0, reflow_time
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0)
    mov a, edit_sett
    cjne a, #2, indic_refl_time
    Set_Cursor(1,6)
    sjmp indic_soak_next
indic_refl_time:
    Set_Cursor(1,15)
indic_refl_next:
    Display_char(#'*')
    ret
coolScreen:
    Set_Cursor(1,1)
    Send_Constant_String(#setup4)
    Set_Cursor(2,1)
    Send_Constant_String(#setup5)

    Set_Cursor(2,5)
    load_X(0)
    mov x+0, cool_temp
    lcall hex2bcd
    ; Display digit 3
    mov a, bcd+1
    anl a, #0fh
    orl a, #'0'
    mov r0, a
    WriteData(r0)
    Display_BCD(bcd+0)
    ret


pollButtons:
    jb EDIT, DONT_EDIT 		
	Wait_Milli_Seconds(#50)		
	jb EDIT, DONT_EDIT
	jnb EDIT, $

    mov a, edit_sett
    cjne a, #4, incEdit
    mov edit_sett, #0
    incEdit: inc_setting(edit_sett)

; 0 - soak temp
; 1 - soak time
; 2 - reflow temp
; 3 - reflow time
; 4 - cool temp   
DONT_EDIT:
    jb INCR, DONT_INC	
	Wait_Milli_Seconds(#50)		
	jb INCR, DONT_INC 		
	jnb INCR, $
    
    mov a, edit_sett
    cjne a, #0, elem1
    inc_setting(soak_temp)
    lcall generateDisplay
    ret
    elem1: cjne a, #1, elem2
    inc_setting(soak_time)
    lcall generateDisplay
    ret
    elem2: cjne a, #2, elem3
    inc_setting(reflow_temp)
    lcall generateDisplay
    ret
    elem3: cjne a, #3, elem4
    inc_setting(reflow_time)
    lcall generateDisplay
    ret
    elem4: inc_setting(cool_temp)
    lcall generateDisplay
    ret
    
DONT_INC:
    jb DECR, DONT_DEC
	Wait_Milli_Seconds(#50)		
	jb DECR, DONT_DEC	
	jnb DECR, $

    mov a, edit_sett
    cjne a, #0, delem1
    dec_setting(soak_temp)
    lcall generateDisplay
    ret
    delem1: cjne a, #1, delem2
    dec_setting(soak_time)
    lcall generateDisplay
    ret
    delem2: cjne a, #2, delem3
    dec_setting(reflow_temp)
    lcall generateDisplay
    ret
    delem3: cjne a, #3, delem4
    dec_setting(reflow_time)
    lcall generateDisplay
    ret
    delem4: dec_setting(cool_temp)
    lcall generateDisplay
    ret

DONT_DEC: ret

;---------------------------------------------------------------------------------------

reset:
	jb RST, DONT_RESET 				; if 'RESET' is pressed, wait for rebouce
	Wait_Milli_Seconds(#50)			; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb RST, DONT_RESET 				; if the 'RESET' button is not pressed skip
	jnb RST, $
	mov a, #5						; reset to state 5 when reset for safety
	mov state, a
DONT_RESET: ret	

start_or_not:
	jb START_STOP, DONT_START 		; if 'RESET' is pressed, wait for rebouce
	Wait_Milli_Seconds(#50)			; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb START_STOP, DONT_START 		; if the 'RESET' button is not pressed skip
	jnb START_STOP, $
	cpl start_flag
	DONT_START: ret	

PWM_OUTPUT:
	mov a, pwm
	cjne a, #100, holding_temp		; if pwm is 100, then OUTPUT = 1 all 
	setb OUTPUT						; the time
	ret

	cjne a, #0, holding_temp		; if pwm is 0, then OUTPUT = 0 all
	clr OUTPUT						; the time
	ret

	holding_temp:	
	mov a, Count1ms
	cjne a, #0 , Not_yet			; check whether it is time to turn on the pwm pin		 
	clr OUTPUT						; clr OUTPUT if at the begining of the period
	mov a, Count1ms+0
	cjne a, #low(PWM_20), Not_yet 	; Warning: this instruction changes the carry flag!
	mov a, Count1ms+1
	cjne a, #high(PWM_20), Not_yet	; if Count1ms = PWM_20, set the OUTPUT to 1
	setb OUTPUT
Not_yet: ret

Load_Defaults: ; Load defaults if 'keys' are incorrect
	mov soak_temp, #150
	mov soak_time, #45
	mov reflow_temp, #225
	mov reflow_time, #30
    mov cool_temp, #50
	ret

;-------------------------------------FSM time!!---------------------------------------
FSM:							 
	mov a, state
state0:							; default state
	cjne a, #0, state1			; if not state 0, then go to next branch
	mov pwm, #0					; at state 0, pwm is 0%
	lcall start_or_not
	jnb start_flag, state0_done	; if start key is not press, the go to state0_done
	mov state, #1
	clr start_flag
state0_done:
	ljmp forever
state1:							; ramp to soak
	cjne a, #1, state2
	mov pwm, #100
	mov sec, #0
	mov a, soak_temp
	clr c
	subb a, temp				; if temp > soak_temp, c = 1
	jnc state1_done				; if temp is not at soak temp, then go to state1_done
	mov state, #2
	setb start_flag
state1_done:
	ljmp forever

state2:							; soak/preheat
	cjne a, #2, state3
	mov pwm, #HOLD_PWM
	mov a, soak_time
	clr c
	subb a, sec					; if sec > soak time, c = 1
	jnc state2_done				; if sec is not at soak time, then go to state2_done 
	mov state, #3	
state2_done:
	ljmp forever

state3:							; ramp to peak, prepare to reflow
	cjne a, #3, state4
	mov pwm, #100
	mov sec, #0
	mov a, reflow_temp
	clr c
	subb a, temp				; if temp > reflow_temp, c = 1
	jnc state3_done				; if temp is not at reflow_temp, then go to state3_done 
	mov state, #4	
state3_done:
	ljmp forever

state4:							; ramp to peak, prepare to reflow
	cjne a, #4, state5
	mov pwm, #HOLD_PWM
	mov a, reflow_time
	clr c
	subb a, sec					; if sec > reflow_temp, c = 1
	jnc state4_done				; if sec is not at reflow time, then go to state4_done 
	mov state, #5	
state4_done:
	ljmp forever

state5:							; cooling state
	cjne a, #5, state0
	mov pwm, #0
	mov a, temp
	clr c
	subb a, cool_temp			; if cool_temp > temp, c = 1
	jnc state5_done				; if temp is not at cool_temp, then go to state5_done 
	mov state, #0	
state5_done:
	ljmp forever 
;----------------------------------------------------------------------------------------

;---------------------------------save to nvmem-------------------------------
loadbyte mac
	mov a, %0
	movx @dptr, a
	inc dptr
endmac
save_config:
    push IE ; Save the current state of bit EA in the stack
    clr EA ; Disable interrupts
	mov FCON, #0x08 ; Page Buffer Mapping Enabled (FPS = 1)
	mov dptr, #0x7f80 ; Last page of flash memory
	; Save variables
	loadbyte(soak_temp) ; @0x7f80
	loadbyte(soak_time) ; @0x7f81
	loadbyte(reflow_temp) ; @0x7f82
	loadbyte(reflow_time) ; @0x7f83
    loadbyte(cool_temp) ; @0x7f84
	loadbyte(#0x55) ; First key value @0x7f84
	loadbyte(#0xAA) ; Second key value @0x7f85
	mov FCON, #0x00 ; Page Buffer Mapping Disabled (FPS = 0)
	orl EECON, #0b01000000 ; Enable auto-erase on next write sequence
	mov FCON, #0x50 ; Write trigger first byte
	mov FCON, #0xA0 ; Write trigger second byte
	; CPU idles until writing of flash completes.
	mov FCON, #0x00 ; Page Buffer Mapping Disabled (FPS = 0)
	anl EECON, #0b10111111 ; Disable auto-erase
	pop IE ; Restore the state of bit EA from the stack
    ret
;-----------------------------------------------------------------------------

;------------------------------read from nvmem--------------------------------
getbyte mac
    clr a
    movc a, @a+dptr
    mov %0, a
    inc dptr
Endmac
Load_Configuration:
    mov dptr, #0x7f84 ; First key value location.
    getbyte(R0) ; 0x7f84 should contain 0x55
    cjne R0, #0x55, Load_Defaults
    getbyte(R0) ; 0x7f85 should contain 0xAA
    cjne R0, #0xAA, Load_Defaults
; Keys are good. Get stored values.
    mov dptr, #0x7f80
    getbyte(soak_temp) ; 0x7f80
    getbyte(soak_time) ; 0x7f81
    getbyte(reflow_temp) ; 0x7f82
    getbyte(reflow_time) ; 0x7f83
    getbyte(cool_temp)
    ret
;----------------------------------------------------------------------------

;----------------------------------------------------------------------
cold_junc:

ret
;----------------------------------------------------------------------


;----------------------------------------------------------------------
hot_junc:

ret
;----------------------------------------------------------------------

END
