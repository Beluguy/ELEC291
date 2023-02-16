; AT89LP51RC2_Receiver.asm:  This program implements a simple serial port
; communication protocol to program, verify, and read an SPI flash memory.  Since
; the program was developed to store wav audio files, it also allows 
; for the playback of said audio.  It is assumed that the wav sampling rate is
; 22050Hz, 8-bit, mono.
;
; Connections:
; 
; AT89LP51RD2   SPI_FLASH
; (20) P2.0     Pin 6 (SPI_CLK)
; (21) P2.1     Pin 2 (MISO)
; (24) P2.4     Pin 5 (MOSI)
; (25) P2.5     Pin 1 (CS/)
; GND           Pin 4
; 3.3V          Pins 3, 7, 8
;
; The DAC output (P2.3, pin 23) should be connected to the
; input of power amplifier (LM386 or similar)
;
; WARNING: Pins P2.2 and P2.3 are the DAC outputs and can not be used for anything else

$NOLIST
$MODLP51RC2
$LIST

SYSCLK         EQU 22118400  ; Microcontroller system clock frequency in Hz
TIMER1_RATE    EQU 22050     ; 22050Hz is the sampling rate of the wav file we are playing
TIMER1_RELOAD  EQU 0x10000-(SYSCLK/TIMER1_RATE)
BAUDRATE       EQU 115200
BRG_VAL        EQU (0x100-(SYSCLK/(16*BAUDRATE)))

SPEAKER  EQU P2.4 ; Used with a MOSFET to turn off speaker when not in use

; The pins used for SPI
FLASH_CE  EQU  P0.7			; 1, was 2.5
MY_MOSI   EQU  P2.5 		; 5, was 2.4
MY_MISO   EQU  P2.7			; 2, was 2.1
MY_SCLK   EQU  P0.4 		; 6, was 2.0

; Commands supported by the SPI flash memory according to the datasheet
WRITE_ENABLE     EQU 0x06  ; Address:0 Dummy:0 Num:0
WRITE_DISABLE    EQU 0x04  ; Address:0 Dummy:0 Num:0
READ_STATUS      EQU 0x05  ; Address:0 Dummy:0 Num:1 to infinite
READ_BYTES       EQU 0x03  ; Address:3 Dummy:0 Num:1 to infinite
READ_SILICON_ID  EQU 0xab  ; Address:0 Dummy:3 Num:1 to infinite
FAST_READ        EQU 0x0b  ; Address:3 Dummy:1 Num:1 to infinite
WRITE_STATUS     EQU 0x01  ; Address:0 Dummy:0 Num:1
WRITE_BYTES      EQU 0x02  ; Address:3 Dummy:0 Num:1 to 256
ERASE_ALL        EQU 0xc7  ; Address:0 Dummy:0 Num:0
ERASE_BLOCK      EQU 0xd8  ; Address:3 Dummy:0 Num:0
READ_DEVICE_ID   EQU 0x9f  ; Address:0 Dummy:2 Num:1 to infinite

; Variables used in the program:
dseg at 30H
	w:   ds 3 ; 24-bit play counter.  Decremented in Timer 1 ISR.
	x:	 ds 3
	n:	 ds 3

bseg
	coolingflag: dbit 1

; Interrupt vectors:
cseg

org 0x0000 ; Reset vector
    ljmp MainProgram

org 0x001B ; Timer/Counter 1 overflow interrupt vector. Used in this code to replay the wave file.
	ljmp Timer1_ISR


org 0x0063 ; ADC interrupt (vector must be present if debugger is used)
	reti
   	
$NOLIST
$include(WaitMilliSeconds.inc)
$include(audiocalls.inc)
$LIST


;AFTER MOVING NEEDED NUMBER
;PLAYAUDIO


;-------------------------------------;
; ISR for Timer 1.  Used to playback  ;
; the WAV file stored in the SPI      ;
; flash memory.                       ;
;-------------------------------------;
Timer1_ISR:
	; The registers used in the ISR must be saved in the stack
	push acc
	push psw
	
	; Check if the play counter is zero.  If so, stop playing sound.
	mov a, w+0
	orl a, w+1
	orl a, w+2
	jz stop_playing
	
	; Decrement play counter 'w'.  In this implementation 'w' is a 24-bit counter.
	mov a, #0xff
	dec w+0
	cjne a, w+0, keep_playing
	dec w+1
	cjne a, w+1, keep_playing
	dec w+2
	
keep_playing:
	setb SPEAKER
	lcall Send_SPI ; Read the next byte from the SPI Flash...
	;mov P0, a ; WARNING: Remove this if not using an external DAC to use the pins of P0 as GPIO
	add a, #0x80
	mov DADH, a ; Output to DAC. DAC output is pin P2.3
	orl DADC, #0b_0100_0000 ; Start DAC by setting GO/BSY=1
	sjmp Timer1_ISR_Done

stop_playing:
	clr TR1 ; Stop timer 1
	setb FLASH_CE  ; Disable SPI Flash
	clr SPEAKER ; Turn off speaker.  Removes hissing noise when not playing sound.
	mov DADH, #0x80 ; middle of range
	orl DADC, #0b_0100_0000 ; Start DAC by setting GO/BSY=1

Timer1_ISR_Done:	
	pop psw
	pop acc
	reti

; Approximate index of sounds in file 'reflow_oven_denoise.wav'
sound_index:
    db 0x00, 0x00, 0x2b ; 0		1
    db 0x00, 0x3b, 0x30 ; 1  	2
    db 0x00, 0x93, 0x84 ; 2  	3
    db 0x00, 0xeb, 0x84 ; 3   	4
    db 0x01, 0x46, 0xb2 ; 4 	5
    db 0x01, 0x9d, 0x2f ; 5 	6
    db 0x01, 0xe3, 0x68 ; 6 	7
    db 0x01, 0xf7, 0xa7 ; 7 	7
    db 0x02, 0x49, 0xc7 ; 8 	8
    db 0x02, 0x8f, 0xc0 ; 9 	9
    db 0x02, 0xf1, 0x59 ; 10 	10
    db 0x03, 0x37, 0x35 ; 11 	20
    db 0x03, 0x89, 0x37 ; 12 	30
    db 0x03, 0xdc, 0xf6 ; 13 	40
    db 0x04, 0x35, 0x90 ; 14 	50
    db 0x04, 0x76, 0xa8 ; 15 	50,60 cuts off
    db 0x04, 0x8f, 0x8c ; 16 	60
    db 0x04, 0xd1, 0x41 ; 17 	cuts off
    db 0x04, 0xe4, 0x5b ; 18 	70
    db 0x05, 0x3a, 0xb8 ; 19 	80
    db 0x05, 0x85, 0x67 ; 20 	90
    db 0x05, 0xdc, 0x0e ; 21 	100 ->2sec from now on
    db 0x06, 0x33, 0xae ; 22 	degree celsius
    db 0x06, 0x3d, 0xed ; 23 	degree celsius
    db 0x06, 0xea, 0x8e ; 24 	current state
    db 0x07, 0x7a, 0x14 ; 25 	ramp to soak
    db 0x07, 0xd3, 0xd0 ; 26 	empty
    db 0x08, 0x01, 0xc6 ; 27 	soak
    db 0x08, 0x0a, 0x39 ; 28 	soak
    db 0x08, 0x70, 0x59 ; 29 	ramp to peak
    db 0x08, 0x7c, 0x2f ; 30 	ramp to peak
    db 0x08, 0xfd, 0xf0 ; 31 	cuts off
    db 0x09, 0x0b, 0xb1 ; 32 	peak - reflow (overlaps)
    db 0x09, 0x23, 0x92 ; 33 	reflow
    db 0x09, 0x2b, 0x19 ; 34 	reflow
    db 0x09, 0xaa, 0xff ; 35 	cooling
    db 0x0a, 0x26, 0xe0 ; 36 	the oven is at a safe temperature
    db 0x0a, 0xb2, 0xa0 ; 37 	the oven is at a safe temperature
    db 0x0a, 0xd6, 0xcd ; 38 	the oven is at a safe temperature
    db 0x0a, 0xe7, 0x36 ; 39 	----(overlaps)
    db 0x0b, 0x07, 0x28 ; 40 	the pcb may still be hot 
    db 0x0b, 0x3a, 0x6c ; 41 	the pcb may still be hot
    db 0x0b, 0xac, 0xa4 ; 42 	empty
    db 0x0b, 0xf6, 0xf0 ; 43	empty

; Size of each sound in 'sound_index'
Size_sound:
    db 0x00, 0x3b, 0x05 ; 0 
    db 0x00, 0x58, 0x54 ; 1 
    db 0x00, 0x58, 0x00 ; 2 
    db 0x00, 0x5b, 0x2e ; 3 
    db 0x00, 0x56, 0x7d ; 4 
    db 0x00, 0x46, 0x39 ; 5 
    db 0x00, 0x14, 0x3f ; 6 
    db 0x00, 0x52, 0x20 ; 7 
    db 0x00, 0x45, 0xf9 ; 8 
    db 0x00, 0x61, 0x99 ; 9 
    db 0x00, 0x45, 0xdc ; 10 
    db 0x00, 0x52, 0x02 ; 11 
    db 0x00, 0x53, 0xbf ; 12 
    db 0x00, 0x58, 0x9a ; 13 
    db 0x00, 0x41, 0x18 ; 14 
    db 0x00, 0x18, 0xe4 ; 15 
    db 0x00, 0x41, 0xb5 ; 16 
    db 0x00, 0x13, 0x1a ; 17 
    db 0x00, 0x56, 0x5d ; 18 
    db 0x00, 0x4a, 0xaf ; 19 
    db 0x00, 0x56, 0xa7 ; 20 
    db 0x00, 0x57, 0xa0 ; 21 
    db 0x00, 0x0a, 0x3f ; 22 
    db 0x00, 0xac, 0xa1 ; 23 
    db 0x00, 0x8f, 0x86 ; 24 
    db 0x00, 0x59, 0xbc ; 25 
    db 0x00, 0x2d, 0xf6 ; 26 
    db 0x00, 0x08, 0x73 ; 27 
    db 0x00, 0x66, 0x20 ; 28 
    db 0x00, 0x0b, 0xd6 ; 29 
    db 0x00, 0x81, 0xc1 ; 30 
    db 0x00, 0x0d, 0xc1 ; 31 
    db 0x00, 0x17, 0xe1 ; 32 
    db 0x00, 0x07, 0x87 ; 33 
    db 0x00, 0x7f, 0xe6 ; 34 
    db 0x00, 0x7b, 0xe1 ; 35 
    db 0x00, 0x8b, 0xc0 ; 36 
    db 0x00, 0x24, 0x2d ; 37 
    db 0x00, 0x10, 0x69 ; 38 
    db 0x00, 0x1f, 0xf2 ; 39 
    db 0x00, 0x33, 0x44 ; 40 
    db 0x00, 0x72, 0x38 ; 41 
    db 0x00, 0x4a, 0x4c ; 42 


;---------------------------------;
; Sends a byte via serial port    ;
;---------------------------------;
putchar:
	jbc	TI,putchar_L1
	sjmp putchar
putchar_L1:
	mov	SBUF,a
	ret

;---------------------------------;
; Receive a byte from serial port ;
;---------------------------------;
getchar:
	jbc	RI,getchar_L1
	sjmp getchar
getchar_L1:
	mov	a,SBUF
	ret

;---------------------------------;
; Sends AND receives a byte via   ;
; SPI.                            ;
;---------------------------------;
Send_SPI:
	SPIBIT MAC
	    ; Send/Receive bit %0
		rlc a
		mov MY_MOSI, c
		setb MY_SCLK
		mov c, MY_MISO
		clr MY_SCLK
		mov acc.0, c
	ENDMAC
	
	SPIBIT(7)
	SPIBIT(6)
	SPIBIT(5)
	SPIBIT(4)
	SPIBIT(3)
	SPIBIT(2)
	SPIBIT(1)
	SPIBIT(0)

	ret

;---------------------------------;
; SPI flash 'write enable'        ;
; instruction.                    ;
;---------------------------------;
Enable_Write:
	clr FLASH_CE
	mov a, #WRITE_ENABLE
	lcall Send_SPI
	setb FLASH_CE
	ret

;---------------------------------;
; This function checks the 'write ;
; in progress' bit of the SPI     ;
; flash memory.                   ;
;---------------------------------;
Check_WIP:
	clr FLASH_CE
	mov a, #READ_STATUS
	lcall Send_SPI
	mov a, #0x55
	lcall Send_SPI
	setb FLASH_CE
	jb acc.0, Check_WIP ;  Check the Write in Progress bit
	ret
	
Init_all:
    ; Since the reset button bounces, we need to wait a bit before
    ; sending messages, otherwise we risk displaying gibberish!
    mov R1, #222
    mov R0, #166
    djnz R0, $   ; 3 cycles->3*45.21123ns*166=22.51519us
    djnz R1, $-4 ; 22.51519us*222=4.998ms
    ; Now we can proceed with the configuration
	
	; Enable serial communication and set up baud rate
	orl	PCON,#0x80
	mov	SCON,#0x52
	mov	BDRCON,#0x00
	mov	BRL,#BRG_VAL
	mov	BDRCON,#0x1E ; BDRCON=BRR|TBCK|RBCK|SPD;
	
	; Configure SPI pins and turn off speaker
	;anl P2M0, #0b_1100_1110
	;orl P2M1, #0b_0011_0001

	; Configure MY_MOSI/P2.5 as open drain output
	orl P2M0, #0b_0010_0000
	orl P2M1, #0b_0010_0000

	; Configure FLASH_CE/P0.7 and MY_SCLK/P0.4 as open drain outputs
	orl P0M0, #0b_1001_0000
	orl P0M1, #0b_1001_000

	setb MY_MISO  ; Configured as input
	setb FLASH_CE ; CS=1 for SPI flash memory
	clr MY_SCLK   ; Rest state of SCLK=0
	clr SPEAKER   ; Turn off speaker.
	
	; Configure timer 1
	anl	TMOD, #0x0F ; Clear the bits of timer 1 in TMOD
	orl	TMOD, #0x10 ; Set timer 1 in 16-bit timer mode.  Don't change the bits of timer 0
	mov TH1, #high(TIMER1_RELOAD)
	mov TL1, #low(TIMER1_RELOAD)
	; Set autoreload value
	mov RH1, #high(TIMER1_RELOAD)
	mov RL1, #low(TIMER1_RELOAD)

	; Enable the timer and interrupts
    setb ET1  ; Enable timer 1 interrupt
	; setb TR1 ; Timer 1 is only enabled to play stored sound

	; Configure the DAC.  The DAC output we are using is P2.3, but P2.2 is also reserved.
	mov DADI, #0b_1010_0000 ; ACON=1
	mov DADC, #0b_0011_1010 ; Enabled, DAC mode, Left adjusted, CLK/4
	mov DADH, #0x80 ; Middle of scale
	mov DADL, #0
	orl DADC, #0b_0100_0000 ; Start DAC by GO/BSY=1
check_DAC_init:
	mov a, DADC
	jb acc.6, check_DAC_init ; Wait for DAC to finish
	
	setb EA ; Enable interrupts

	; Not necesary if using internal DAC.
	; If using an R-2R DAC connected to P0, configure the pins of P0
	; (An external R-2R produces much better quality sound)
	;mov P0M0, #0b_0000_0000
	;mov P0M1, #0b_1111_1111
	
	ret

;---------------------------------;
; CRC-CCITT (XModem) Polynomial:  ;
; x^16 + x^12 + x^5 + 1 (0x1021)  ;
; CRC in [R7,R6].                 ;
; Converted to a macro to remove  ;
; the overhead of 'lcall' and     ;
; 'ret' instructions, since this  ;
; 'routine' may be executed over  ;
; 4 million times!                ;
;---------------------------------;
;crc16:
crc16 mac
	xrl	a, r7			; XOR high of CRC with byte
	mov r0, a			; Save for later use
	mov	dptr, #CRC16_TH ; dptr points to table high
	movc a, @a+dptr		; Get high part from table
	xrl	a, r6			; XOR With low byte of CRC
	mov	r7, a			; Store to high byte of CRC
	mov a, r0			; Retrieve saved accumulator
	mov	dptr, #CRC16_TL	; dptr points to table low	
	movc a, @a+dptr		; Get Low from table
	mov	r6, a			; Store to low byte of CRC
	;ret
endmac

;---------------------------------;
; High constants for CRC-CCITT    ;
; (XModem) Polynomial:            ;
; x^16 + x^12 + x^5 + 1 (0x1021)  ;
;---------------------------------;
CRC16_TH:
	db	000h, 010h, 020h, 030h, 040h, 050h, 060h, 070h
	db	081h, 091h, 0A1h, 0B1h, 0C1h, 0D1h, 0E1h, 0F1h
	db	012h, 002h, 032h, 022h, 052h, 042h, 072h, 062h
	db	093h, 083h, 0B3h, 0A3h, 0D3h, 0C3h, 0F3h, 0E3h
	db	024h, 034h, 004h, 014h, 064h, 074h, 044h, 054h
	db	0A5h, 0B5h, 085h, 095h, 0E5h, 0F5h, 0C5h, 0D5h
	db	036h, 026h, 016h, 006h, 076h, 066h, 056h, 046h
	db	0B7h, 0A7h, 097h, 087h, 0F7h, 0E7h, 0D7h, 0C7h
	db	048h, 058h, 068h, 078h, 008h, 018h, 028h, 038h
	db	0C9h, 0D9h, 0E9h, 0F9h, 089h, 099h, 0A9h, 0B9h
	db	05Ah, 04Ah, 07Ah, 06Ah, 01Ah, 00Ah, 03Ah, 02Ah
	db	0DBh, 0CBh, 0FBh, 0EBh, 09Bh, 08Bh, 0BBh, 0ABh
	db	06Ch, 07Ch, 04Ch, 05Ch, 02Ch, 03Ch, 00Ch, 01Ch
	db	0EDh, 0FDh, 0CDh, 0DDh, 0ADh, 0BDh, 08Dh, 09Dh
	db	07Eh, 06Eh, 05Eh, 04Eh, 03Eh, 02Eh, 01Eh, 00Eh
	db	0FFh, 0EFh, 0DFh, 0CFh, 0BFh, 0AFh, 09Fh, 08Fh
	db	091h, 081h, 0B1h, 0A1h, 0D1h, 0C1h, 0F1h, 0E1h
	db	010h, 000h, 030h, 020h, 050h, 040h, 070h, 060h
	db	083h, 093h, 0A3h, 0B3h, 0C3h, 0D3h, 0E3h, 0F3h
	db	002h, 012h, 022h, 032h, 042h, 052h, 062h, 072h
	db	0B5h, 0A5h, 095h, 085h, 0F5h, 0E5h, 0D5h, 0C5h
	db	034h, 024h, 014h, 004h, 074h, 064h, 054h, 044h
	db	0A7h, 0B7h, 087h, 097h, 0E7h, 0F7h, 0C7h, 0D7h
	db	026h, 036h, 006h, 016h, 066h, 076h, 046h, 056h
	db	0D9h, 0C9h, 0F9h, 0E9h, 099h, 089h, 0B9h, 0A9h
	db	058h, 048h, 078h, 068h, 018h, 008h, 038h, 028h
	db	0CBh, 0DBh, 0EBh, 0FBh, 08Bh, 09Bh, 0ABh, 0BBh
	db	04Ah, 05Ah, 06Ah, 07Ah, 00Ah, 01Ah, 02Ah, 03Ah
	db	0FDh, 0EDh, 0DDh, 0CDh, 0BDh, 0ADh, 09Dh, 08Dh
	db	07Ch, 06Ch, 05Ch, 04Ch, 03Ch, 02Ch, 01Ch, 00Ch
	db	0EFh, 0FFh, 0CFh, 0DFh, 0AFh, 0BFh, 08Fh, 09Fh
	db	06Eh, 07Eh, 04Eh, 05Eh, 02Eh, 03Eh, 00Eh, 01Eh

;---------------------------------;
; Low constants for CRC-CCITT     ;
; (XModem) Polynomial:            ;
; x^16 + x^12 + x^5 + 1 (0x1021)  ;
;---------------------------------;
CRC16_TL:
	db	000h, 021h, 042h, 063h, 084h, 0A5h, 0C6h, 0E7h
	db	008h, 029h, 04Ah, 06Bh, 08Ch, 0ADh, 0CEh, 0EFh
	db	031h, 010h, 073h, 052h, 0B5h, 094h, 0F7h, 0D6h
	db	039h, 018h, 07Bh, 05Ah, 0BDh, 09Ch, 0FFh, 0DEh
	db	062h, 043h, 020h, 001h, 0E6h, 0C7h, 0A4h, 085h
	db	06Ah, 04Bh, 028h, 009h, 0EEh, 0CFh, 0ACh, 08Dh
	db	053h, 072h, 011h, 030h, 0D7h, 0F6h, 095h, 0B4h
	db	05Bh, 07Ah, 019h, 038h, 0DFh, 0FEh, 09Dh, 0BCh
	db	0C4h, 0E5h, 086h, 0A7h, 040h, 061h, 002h, 023h
	db	0CCh, 0EDh, 08Eh, 0AFh, 048h, 069h, 00Ah, 02Bh
	db	0F5h, 0D4h, 0B7h, 096h, 071h, 050h, 033h, 012h
	db	0FDh, 0DCh, 0BFh, 09Eh, 079h, 058h, 03Bh, 01Ah
	db	0A6h, 087h, 0E4h, 0C5h, 022h, 003h, 060h, 041h
	db	0AEh, 08Fh, 0ECh, 0CDh, 02Ah, 00Bh, 068h, 049h
	db	097h, 0B6h, 0D5h, 0F4h, 013h, 032h, 051h, 070h
	db	09Fh, 0BEh, 0DDh, 0FCh, 01Bh, 03Ah, 059h, 078h
	db	088h, 0A9h, 0CAh, 0EBh, 00Ch, 02Dh, 04Eh, 06Fh
	db	080h, 0A1h, 0C2h, 0E3h, 004h, 025h, 046h, 067h
	db	0B9h, 098h, 0FBh, 0DAh, 03Dh, 01Ch, 07Fh, 05Eh
	db	0B1h, 090h, 0F3h, 0D2h, 035h, 014h, 077h, 056h
	db	0EAh, 0CBh, 0A8h, 089h, 06Eh, 04Fh, 02Ch, 00Dh
	db	0E2h, 0C3h, 0A0h, 081h, 066h, 047h, 024h, 005h
	db	0DBh, 0FAh, 099h, 0B8h, 05Fh, 07Eh, 01Dh, 03Ch
	db	0D3h, 0F2h, 091h, 0B0h, 057h, 076h, 015h, 034h
	db	04Ch, 06Dh, 00Eh, 02Fh, 0C8h, 0E9h, 08Ah, 0ABh
	db	044h, 065h, 006h, 027h, 0C0h, 0E1h, 082h, 0A3h
	db	07Dh, 05Ch, 03Fh, 01Eh, 0F9h, 0D8h, 0BBh, 09Ah
	db	075h, 054h, 037h, 016h, 0F1h, 0D0h, 0B3h, 092h
	db	02Eh, 00Fh, 06Ch, 04Dh, 0AAh, 08Bh, 0E8h, 0C9h
	db	026h, 007h, 064h, 045h, 0A2h, 083h, 0E0h, 0C1h
	db	01Fh, 03Eh, 05Dh, 07Ch, 09Bh, 0BAh, 0D9h, 0F8h
	db	017h, 036h, 055h, 074h, 093h, 0B2h, 0D1h, 0F0h

;---------------------------------;
; Main program. Includes hardware ;
; initialization and 'forever'    ;
; loop.                           ;
;---------------------------------;
MainProgram:
    mov SP, #0x7f ; Setup stack pointer to the start of indirectly accessable data memory minus one
    lcall Init_all ; Initialize the hardware  
    ; In case you decide to use the pins of P0, configure the port in bidirectional mode:
    mov P0M0, #0
    mov P0M1, #0
	setb coolingflag
	clr coolingflag
    
forever_loop:

		

	;saLoad_X(0)
	jb RI, serial_get
	
	jb P4.5, forever_loop ; Check if push-button pressed
	Wait_Milli_Seconds(#50)	; Debounce delay.  This macro is also in 'LCD_4bit.inc'
	jb p4.5, forever_loop  ; if the 'BOOT' button is not pressed skip
	jnb p4.5, $ 
	
	
	
	
	;jnb P4.5, $ ; Wait for push-button release
	; Play the whole memory
	clr TR1 ; Stop Timer 1 ISR from playing previous request
	setb FLASH_CE
	clr SPEAKER ; Turn off speaker.
	
	clr FLASH_CE ; Enable SPI Flash
	mov a, #READ_BYTES
	lcall Send_SPI
	
	;mov dptr, #0x00 
	;mov R0, #0x009384
	
	;mov a, #1
	;add a, #-1
	;mov b, #3
	;mul ab
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	jnb coolingflag, next
	lcall cooling
	
next:
	
	load_X(100)
	lcall main_player_1sec
	
	;lcall cooling
	;lcall safe_temp
	
	;load_X(100)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;Wait_Milli_Seconds(#250)
	;load_X(5)
	;lcall main_player_1sec
	;Wait_Milli_Seconds(#250)
		
	setb SPEAKER ; Turn on speaker.
	setb TR1 ; Start playback by enabling Timer 1
	
	
	ljmp forever_loop
	
serial_get:
	lcall getchar ; Wait for data to arrive
	cjne a, #'#', forever_loop ; Message format is #n[data] where 'n' is '0' to '9'
	clr TR1 ; Stop Timer 1 from playing previous request
	setb FLASH_CE ; Disable SPI Flash	
	clr SPEAKER ; Turn off speaker.
	lcall getchar

;---------------------------------------------------------	
	cjne a, #'0' , Command_0_skip
Command_0_start: ; Identify command
	clr FLASH_CE ; Enable SPI Flash	
	mov a, #READ_DEVICE_ID
	lcall Send_SPI	
	mov a, #0x55
	lcall Send_SPI
	lcall putchar
	mov a, #0x55
	lcall Send_SPI
	lcall putchar
	mov a, #0x55
	lcall Send_SPI
	lcall putchar
	setb FLASH_CE ; Disable SPI Flash
	ljmp forever_loop	
Command_0_skip:

;---------------------------------------------------------	
	cjne a, #'1' , Command_1_skip 
Command_1_start: ; Erase whole flash (takes a long time)
	lcall Enable_Write
	clr FLASH_CE
	mov a, #ERASE_ALL
	lcall Send_SPI
	setb FLASH_CE
	lcall Check_WIP
	mov a, #0x01 ; Send 'I am done' reply
	lcall putchar		
	ljmp forever_loop	
Command_1_skip:

;---------------------------------------------------------	
	cjne a, #'2' , Command_2_skip 
Command_2_start: ; Load flash page (256 bytes or less)
	lcall Enable_Write
	clr FLASH_CE
	mov a, #WRITE_BYTES
	lcall Send_SPI
	lcall getchar ; Address bits 16 to 23
	lcall Send_SPI
	lcall getchar ; Address bits 8 to 15
	lcall Send_SPI
	lcall getchar ; Address bits 0 to 7
	lcall Send_SPI
	lcall getchar ; Number of bytes to write (0 means 256 bytes)
	mov r0, a
Command_2_loop:
	lcall getchar
	lcall Send_SPI
	djnz r0, Command_2_loop
	setb FLASH_CE
	lcall Check_WIP
	mov a, #0x01 ; Send 'I am done' reply
	lcall putchar		
	ljmp forever_loop	
Command_2_skip:

;---------------------------------------------------------	
	cjne a, #'3' , Command_3_skip 
Command_3_start: ; Read flash bytes (256 bytes or less)
	clr FLASH_CE
	mov a, #READ_BYTES
	lcall Send_SPI
	lcall getchar ; Address bits 16 to 23
	lcall Send_SPI
	lcall getchar ; Address bits 8 to 15
	lcall Send_SPI
	lcall getchar ; Address bits 0 to 7
	lcall Send_SPI
	lcall getchar ; Number of bytes to read and send back (0 means 256 bytes)
	mov r0, a

Command_3_loop:
	mov a, #0x55
	lcall Send_SPI
	lcall putchar
	djnz r0, Command_3_loop
	setb FLASH_CE	
	ljmp forever_loop	
Command_3_skip:

;---------------------------------------------------------	
	cjne a, #'4' , Command_4_skip 
Command_4_start: ; Playback a portion of the stored wav file
	clr TR1 ; Stop Timer 1 ISR from playing previous request
	setb FLASH_CE
	nop
	clr FLASH_CE ; Enable SPI Flash
	mov a, #READ_BYTES
	lcall Send_SPI
	; Get the initial position in memory where to start playing
	lcall getchar
	lcall Send_SPI
	lcall getchar
	lcall Send_SPI
	lcall getchar
	lcall Send_SPI
	; Get how many bytes to play
	lcall getchar
	mov w+2, a
	lcall getchar
	mov w+1, a
	lcall getchar
	mov w+0, a
	
	mov a, #0x00 ; Request first byte to send to DAC
	lcall Send_SPI
	
	setb TR1 ; Start playback by enabling timer 1
	ljmp forever_loop	
Command_4_skip:

;---------------------------------------------------------	
	cjne a, #'5' , Command_5_skip 
Command_5_start: ; Calculate and send CRC-16 of ISP flash memory from zero to the 24-bit passed value.
	; Get how many bytes to use to calculate the CRC.  Store in [r5,r4,r3]
	lcall getchar
	mov r5, a
	lcall getchar
	mov r4, a
	lcall getchar
	mov r3, a
	
	; Since we are using the 'djnz' instruction to check, we need to add one to each byte of the counter.
	; A side effect is that the down counter becomes efectively a 23-bit counter, but that is ok
	; because the max size of the 25Q32 SPI flash memory is 400000H.
	inc r3
	inc r4
	inc r5
	
	; Initial CRC must be zero.  Using [r7,r6] to store CRC.
	clr a
	mov r7, a
	mov r6, a

	clr FLASH_CE
	mov a, #READ_BYTES
	lcall Send_SPI
	clr a ; Address bits 16 to 23
	lcall Send_SPI
	clr a ; Address bits 8 to 15
	lcall Send_SPI
	clr a ; Address bits 0 to 7
	lcall Send_SPI
	sjmp Command_5_loop_start

Command_5_loop:
	lcall Send_SPI
	crc16() ; Calculate CRC with new byte
Command_5_loop_start:
	; Drecrement counter:
	djnz r3, Command_5_loop
	djnz r4, Command_5_loop
	djnz r5, Command_5_loop
	
	setb FLASH_CE ; Done reading from SPI flash
	
	; Computation of CRC is complete.  Send 16-bit result using the serial port
	mov a, r7
	lcall putchar
	mov a, r6
	lcall putchar

	ljmp forever_loop	
Command_5_skip:

;---------------------------------------------------------	
	cjne a, #'6' , Command_6_skip 
Command_6_start: ; Fill flash page (256 bytes)
	lcall Enable_Write
	clr FLASH_CE
	mov a, #WRITE_BYTES
	lcall Send_SPI
	lcall getchar ; Address bits 16 to 23
	lcall Send_SPI
	lcall getchar ; Address bits 8 to 15
	lcall Send_SPI
	lcall getchar ; Address bits 0 to 7
	lcall Send_SPI
	lcall getchar ; Byte to write
	mov r1, a
	mov r0, #0 ; 256 bytes
Command_6_loop:
	mov a, r1
	lcall Send_SPI
	djnz r0, Command_6_loop
	setb FLASH_CE
	lcall Check_WIP
	mov a, #0x01 ; Send 'I am done' reply
	lcall putchar		
	ljmp forever_loop	
Command_6_skip:

	ljmp forever_loop

END
