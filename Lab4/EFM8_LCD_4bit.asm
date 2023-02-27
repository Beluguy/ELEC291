;--------------------------------------------------------
; File Created by C51
; Version 1.0.0 #1170 (Feb 16 2022) (MSVC)
; This file was generated Sat Feb 25 15:49:31 2023
;--------------------------------------------------------
$name EFM8_LCD_4bit
$optc51 --model-small
	R_DSEG    segment data
	R_CSEG    segment code
	R_BSEG    segment bit
	R_XSEG    segment xdata
	R_PSEG    segment xdata
	R_ISEG    segment idata
	R_OSEG    segment data overlay
	BIT_BANK  segment data overlay
	R_HOME    segment code
	R_GSINIT  segment code
	R_IXSEG   segment xdata
	R_CONST   segment code
	R_XINIT   segment code
	R_DINIT   segment code

;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	public _main
	public _getsn
	public _LCDprint
	public _LCD_4BIT
	public _WriteCommand
	public _WriteData
	public _LCD_byte
	public _LCD_pulse
	public _waitms
	public _Timer3us
	public __c51_external_startup
	public _LCDprint_PARM_3
	public _getsn_PARM_2
	public _LCDprint_PARM_2
;--------------------------------------------------------
; Special Function Registers
;--------------------------------------------------------
_ACC            DATA 0xe0
_ADC0ASAH       DATA 0xb6
_ADC0ASAL       DATA 0xb5
_ADC0ASCF       DATA 0xa1
_ADC0ASCT       DATA 0xc7
_ADC0CF0        DATA 0xbc
_ADC0CF1        DATA 0xb9
_ADC0CF2        DATA 0xdf
_ADC0CN0        DATA 0xe8
_ADC0CN1        DATA 0xb2
_ADC0CN2        DATA 0xb3
_ADC0GTH        DATA 0xc4
_ADC0GTL        DATA 0xc3
_ADC0H          DATA 0xbe
_ADC0L          DATA 0xbd
_ADC0LTH        DATA 0xc6
_ADC0LTL        DATA 0xc5
_ADC0MX         DATA 0xbb
_B              DATA 0xf0
_CKCON0         DATA 0x8e
_CKCON1         DATA 0xa6
_CLEN0          DATA 0xc6
_CLIE0          DATA 0xc7
_CLIF0          DATA 0xe8
_CLKSEL         DATA 0xa9
_CLOUT0         DATA 0xd1
_CLU0CF         DATA 0xb1
_CLU0FN         DATA 0xaf
_CLU0MX         DATA 0x84
_CLU1CF         DATA 0xb3
_CLU1FN         DATA 0xb2
_CLU1MX         DATA 0x85
_CLU2CF         DATA 0xb6
_CLU2FN         DATA 0xb5
_CLU2MX         DATA 0x91
_CLU3CF         DATA 0xbf
_CLU3FN         DATA 0xbe
_CLU3MX         DATA 0xae
_CMP0CN0        DATA 0x9b
_CMP0CN1        DATA 0x99
_CMP0MD         DATA 0x9d
_CMP0MX         DATA 0x9f
_CMP1CN0        DATA 0xbf
_CMP1CN1        DATA 0xac
_CMP1MD         DATA 0xab
_CMP1MX         DATA 0xaa
_CRC0CN0        DATA 0xce
_CRC0CN1        DATA 0x86
_CRC0CNT        DATA 0xd3
_CRC0DAT        DATA 0xcb
_CRC0FLIP       DATA 0xcf
_CRC0IN         DATA 0xca
_CRC0ST         DATA 0xd2
_DAC0CF0        DATA 0x91
_DAC0CF1        DATA 0x92
_DAC0H          DATA 0x85
_DAC0L          DATA 0x84
_DAC1CF0        DATA 0x93
_DAC1CF1        DATA 0x94
_DAC1H          DATA 0x8a
_DAC1L          DATA 0x89
_DAC2CF0        DATA 0x95
_DAC2CF1        DATA 0x96
_DAC2H          DATA 0x8c
_DAC2L          DATA 0x8b
_DAC3CF0        DATA 0x9a
_DAC3CF1        DATA 0x9c
_DAC3H          DATA 0x8e
_DAC3L          DATA 0x8d
_DACGCF0        DATA 0x88
_DACGCF1        DATA 0x98
_DACGCF2        DATA 0xa2
_DERIVID        DATA 0xad
_DEVICEID       DATA 0xb5
_DPH            DATA 0x83
_DPL            DATA 0x82
_EIE1           DATA 0xe6
_EIE2           DATA 0xf3
_EIP1           DATA 0xbb
_EIP1H          DATA 0xee
_EIP2           DATA 0xed
_EIP2H          DATA 0xf6
_EMI0CN         DATA 0xe7
_FLKEY          DATA 0xb7
_HFO0CAL        DATA 0xc7
_HFO1CAL        DATA 0xd6
_HFOCN          DATA 0xef
_I2C0ADM        DATA 0xff
_I2C0CN0        DATA 0xba
_I2C0DIN        DATA 0xbc
_I2C0DOUT       DATA 0xbb
_I2C0FCN0       DATA 0xad
_I2C0FCN1       DATA 0xab
_I2C0FCT        DATA 0xf5
_I2C0SLAD       DATA 0xbd
_I2C0STAT       DATA 0xb9
_IE             DATA 0xa8
_IP             DATA 0xb8
_IPH            DATA 0xf2
_IT01CF         DATA 0xe4
_LFO0CN         DATA 0xb1
_P0             DATA 0x80
_P0MASK         DATA 0xfe
_P0MAT          DATA 0xfd
_P0MDIN         DATA 0xf1
_P0MDOUT        DATA 0xa4
_P0SKIP         DATA 0xd4
_P1             DATA 0x90
_P1MASK         DATA 0xee
_P1MAT          DATA 0xed
_P1MDIN         DATA 0xf2
_P1MDOUT        DATA 0xa5
_P1SKIP         DATA 0xd5
_P2             DATA 0xa0
_P2MASK         DATA 0xfc
_P2MAT          DATA 0xfb
_P2MDIN         DATA 0xf3
_P2MDOUT        DATA 0xa6
_P2SKIP         DATA 0xcc
_P3             DATA 0xb0
_P3MDIN         DATA 0xf4
_P3MDOUT        DATA 0x9c
_PCA0CENT       DATA 0x9e
_PCA0CLR        DATA 0x9c
_PCA0CN0        DATA 0xd8
_PCA0CPH0       DATA 0xfc
_PCA0CPH1       DATA 0xea
_PCA0CPH2       DATA 0xec
_PCA0CPH3       DATA 0xf5
_PCA0CPH4       DATA 0x85
_PCA0CPH5       DATA 0xde
_PCA0CPL0       DATA 0xfb
_PCA0CPL1       DATA 0xe9
_PCA0CPL2       DATA 0xeb
_PCA0CPL3       DATA 0xf4
_PCA0CPL4       DATA 0x84
_PCA0CPL5       DATA 0xdd
_PCA0CPM0       DATA 0xda
_PCA0CPM1       DATA 0xdb
_PCA0CPM2       DATA 0xdc
_PCA0CPM3       DATA 0xae
_PCA0CPM4       DATA 0xaf
_PCA0CPM5       DATA 0xcc
_PCA0H          DATA 0xfa
_PCA0L          DATA 0xf9
_PCA0MD         DATA 0xd9
_PCA0POL        DATA 0x96
_PCA0PWM        DATA 0xf7
_PCON0          DATA 0x87
_PCON1          DATA 0xcd
_PFE0CN         DATA 0xc1
_PRTDRV         DATA 0xf6
_PSCTL          DATA 0x8f
_PSTAT0         DATA 0xaa
_PSW            DATA 0xd0
_REF0CN         DATA 0xd1
_REG0CN         DATA 0xc9
_REVID          DATA 0xb6
_RSTSRC         DATA 0xef
_SBCON1         DATA 0x94
_SBRLH1         DATA 0x96
_SBRLL1         DATA 0x95
_SBUF           DATA 0x99
_SBUF0          DATA 0x99
_SBUF1          DATA 0x92
_SCON           DATA 0x98
_SCON0          DATA 0x98
_SCON1          DATA 0xc8
_SFRPAGE        DATA 0xa7
_SFRPGCN        DATA 0xbc
_SFRSTACK       DATA 0xd7
_SMB0ADM        DATA 0xd6
_SMB0ADR        DATA 0xd7
_SMB0CF         DATA 0xc1
_SMB0CN0        DATA 0xc0
_SMB0DAT        DATA 0xc2
_SMB0FCN0       DATA 0xc3
_SMB0FCN1       DATA 0xc4
_SMB0FCT        DATA 0xef
_SMB0RXLN       DATA 0xc5
_SMB0TC         DATA 0xac
_SMOD1          DATA 0x93
_SP             DATA 0x81
_SPI0CFG        DATA 0xa1
_SPI0CKR        DATA 0xa2
_SPI0CN0        DATA 0xf8
_SPI0DAT        DATA 0xa3
_SPI0FCN0       DATA 0x9a
_SPI0FCN1       DATA 0x9b
_SPI0FCT        DATA 0xf7
_SPI0PCF        DATA 0xdf
_TCON           DATA 0x88
_TH0            DATA 0x8c
_TH1            DATA 0x8d
_TL0            DATA 0x8a
_TL1            DATA 0x8b
_TMOD           DATA 0x89
_TMR2CN0        DATA 0xc8
_TMR2CN1        DATA 0xfd
_TMR2H          DATA 0xcf
_TMR2L          DATA 0xce
_TMR2RLH        DATA 0xcb
_TMR2RLL        DATA 0xca
_TMR3CN0        DATA 0x91
_TMR3CN1        DATA 0xfe
_TMR3H          DATA 0x95
_TMR3L          DATA 0x94
_TMR3RLH        DATA 0x93
_TMR3RLL        DATA 0x92
_TMR4CN0        DATA 0x98
_TMR4CN1        DATA 0xff
_TMR4H          DATA 0xa5
_TMR4L          DATA 0xa4
_TMR4RLH        DATA 0xa3
_TMR4RLL        DATA 0xa2
_TMR5CN0        DATA 0xc0
_TMR5CN1        DATA 0xf1
_TMR5H          DATA 0xd5
_TMR5L          DATA 0xd4
_TMR5RLH        DATA 0xd3
_TMR5RLL        DATA 0xd2
_UART0PCF       DATA 0xd9
_UART1FCN0      DATA 0x9d
_UART1FCN1      DATA 0xd8
_UART1FCT       DATA 0xfa
_UART1LIN       DATA 0x9e
_UART1PCF       DATA 0xda
_VDM0CN         DATA 0xff
_WDTCN          DATA 0x97
_XBR0           DATA 0xe1
_XBR1           DATA 0xe2
_XBR2           DATA 0xe3
_XOSC0CN        DATA 0x86
_DPTR           DATA 0x8382
_TMR2RL         DATA 0xcbca
_TMR3RL         DATA 0x9392
_TMR4RL         DATA 0xa3a2
_TMR5RL         DATA 0xd3d2
_TMR0           DATA 0x8c8a
_TMR1           DATA 0x8d8b
_TMR2           DATA 0xcfce
_TMR3           DATA 0x9594
_TMR4           DATA 0xa5a4
_TMR5           DATA 0xd5d4
_SBRL1          DATA 0x9695
_PCA0           DATA 0xfaf9
_PCA0CP0        DATA 0xfcfb
_PCA0CP1        DATA 0xeae9
_PCA0CP2        DATA 0xeceb
_PCA0CP3        DATA 0xf5f4
_PCA0CP4        DATA 0x8584
_PCA0CP5        DATA 0xdedd
_ADC0ASA        DATA 0xb6b5
_ADC0GT         DATA 0xc4c3
_ADC0           DATA 0xbebd
_ADC0LT         DATA 0xc6c5
_DAC0           DATA 0x8584
_DAC1           DATA 0x8a89
_DAC2           DATA 0x8c8b
_DAC3           DATA 0x8e8d
;--------------------------------------------------------
; special function bits
;--------------------------------------------------------
_ACC_0          BIT 0xe0
_ACC_1          BIT 0xe1
_ACC_2          BIT 0xe2
_ACC_3          BIT 0xe3
_ACC_4          BIT 0xe4
_ACC_5          BIT 0xe5
_ACC_6          BIT 0xe6
_ACC_7          BIT 0xe7
_TEMPE          BIT 0xe8
_ADGN0          BIT 0xe9
_ADGN1          BIT 0xea
_ADWINT         BIT 0xeb
_ADBUSY         BIT 0xec
_ADINT          BIT 0xed
_IPOEN          BIT 0xee
_ADEN           BIT 0xef
_B_0            BIT 0xf0
_B_1            BIT 0xf1
_B_2            BIT 0xf2
_B_3            BIT 0xf3
_B_4            BIT 0xf4
_B_5            BIT 0xf5
_B_6            BIT 0xf6
_B_7            BIT 0xf7
_C0FIF          BIT 0xe8
_C0RIF          BIT 0xe9
_C1FIF          BIT 0xea
_C1RIF          BIT 0xeb
_C2FIF          BIT 0xec
_C2RIF          BIT 0xed
_C3FIF          BIT 0xee
_C3RIF          BIT 0xef
_D1SRC0         BIT 0x88
_D1SRC1         BIT 0x89
_D1AMEN         BIT 0x8a
_D01REFSL       BIT 0x8b
_D3SRC0         BIT 0x8c
_D3SRC1         BIT 0x8d
_D3AMEN         BIT 0x8e
_D23REFSL       BIT 0x8f
_D0UDIS         BIT 0x98
_D1UDIS         BIT 0x99
_D2UDIS         BIT 0x9a
_D3UDIS         BIT 0x9b
_EX0            BIT 0xa8
_ET0            BIT 0xa9
_EX1            BIT 0xaa
_ET1            BIT 0xab
_ES0            BIT 0xac
_ET2            BIT 0xad
_ESPI0          BIT 0xae
_EA             BIT 0xaf
_PX0            BIT 0xb8
_PT0            BIT 0xb9
_PX1            BIT 0xba
_PT1            BIT 0xbb
_PS0            BIT 0xbc
_PT2            BIT 0xbd
_PSPI0          BIT 0xbe
_P0_0           BIT 0x80
_P0_1           BIT 0x81
_P0_2           BIT 0x82
_P0_3           BIT 0x83
_P0_4           BIT 0x84
_P0_5           BIT 0x85
_P0_6           BIT 0x86
_P0_7           BIT 0x87
_P1_0           BIT 0x90
_P1_1           BIT 0x91
_P1_2           BIT 0x92
_P1_3           BIT 0x93
_P1_4           BIT 0x94
_P1_5           BIT 0x95
_P1_6           BIT 0x96
_P1_7           BIT 0x97
_P2_0           BIT 0xa0
_P2_1           BIT 0xa1
_P2_2           BIT 0xa2
_P2_3           BIT 0xa3
_P2_4           BIT 0xa4
_P2_5           BIT 0xa5
_P2_6           BIT 0xa6
_P3_0           BIT 0xb0
_P3_1           BIT 0xb1
_P3_2           BIT 0xb2
_P3_3           BIT 0xb3
_P3_4           BIT 0xb4
_P3_7           BIT 0xb7
_CCF0           BIT 0xd8
_CCF1           BIT 0xd9
_CCF2           BIT 0xda
_CCF3           BIT 0xdb
_CCF4           BIT 0xdc
_CCF5           BIT 0xdd
_CR             BIT 0xde
_CF             BIT 0xdf
_PARITY         BIT 0xd0
_F1             BIT 0xd1
_OV             BIT 0xd2
_RS0            BIT 0xd3
_RS1            BIT 0xd4
_F0             BIT 0xd5
_AC             BIT 0xd6
_CY             BIT 0xd7
_RI             BIT 0x98
_TI             BIT 0x99
_RB8            BIT 0x9a
_TB8            BIT 0x9b
_REN            BIT 0x9c
_CE             BIT 0x9d
_SMODE          BIT 0x9e
_RI1            BIT 0xc8
_TI1            BIT 0xc9
_RBX1           BIT 0xca
_TBX1           BIT 0xcb
_REN1           BIT 0xcc
_PERR1          BIT 0xcd
_OVR1           BIT 0xce
_SI             BIT 0xc0
_ACK            BIT 0xc1
_ARBLOST        BIT 0xc2
_ACKRQ          BIT 0xc3
_STO            BIT 0xc4
_STA            BIT 0xc5
_TXMODE         BIT 0xc6
_MASTER         BIT 0xc7
_SPIEN          BIT 0xf8
_TXNF           BIT 0xf9
_NSSMD0         BIT 0xfa
_NSSMD1         BIT 0xfb
_RXOVRN         BIT 0xfc
_MODF           BIT 0xfd
_WCOL           BIT 0xfe
_SPIF           BIT 0xff
_IT0            BIT 0x88
_IE0            BIT 0x89
_IT1            BIT 0x8a
_IE1            BIT 0x8b
_TR0            BIT 0x8c
_TF0            BIT 0x8d
_TR1            BIT 0x8e
_TF1            BIT 0x8f
_T2XCLK0        BIT 0xc8
_T2XCLK1        BIT 0xc9
_TR2            BIT 0xca
_T2SPLIT        BIT 0xcb
_TF2CEN         BIT 0xcc
_TF2LEN         BIT 0xcd
_TF2L           BIT 0xce
_TF2H           BIT 0xcf
_T4XCLK0        BIT 0x98
_T4XCLK1        BIT 0x99
_TR4            BIT 0x9a
_T4SPLIT        BIT 0x9b
_TF4CEN         BIT 0x9c
_TF4LEN         BIT 0x9d
_TF4L           BIT 0x9e
_TF4H           BIT 0x9f
_T5XCLK0        BIT 0xc0
_T5XCLK1        BIT 0xc1
_TR5            BIT 0xc2
_T5SPLIT        BIT 0xc3
_TF5CEN         BIT 0xc4
_TF5LEN         BIT 0xc5
_TF5L           BIT 0xc6
_TF5H           BIT 0xc7
_RIE            BIT 0xd8
_RXTO0          BIT 0xd9
_RXTO1          BIT 0xda
_RFRQ           BIT 0xdb
_TIE            BIT 0xdc
_TXHOLD         BIT 0xdd
_TXNF1          BIT 0xde
_TFRQ           BIT 0xdf
;--------------------------------------------------------
; overlayable register banks
;--------------------------------------------------------
	rbank0 segment data overlay
;--------------------------------------------------------
; internal ram data
;--------------------------------------------------------
	rseg R_DSEG
_LCDprint_PARM_2:
	ds 1
_getsn_PARM_2:
	ds 2
_getsn_buff_1_42:
	ds 3
_getsn_sloc0_1_0:
	ds 2
_main_buff_1_48:
	ds 17
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
	rseg	R_OSEG
;--------------------------------------------------------
; indirectly addressable internal ram data
;--------------------------------------------------------
	rseg R_ISEG
;--------------------------------------------------------
; absolute internal ram data
;--------------------------------------------------------
	DSEG
;--------------------------------------------------------
; bit data
;--------------------------------------------------------
	rseg R_BSEG
_LCDprint_PARM_3:
	DBIT	1
;--------------------------------------------------------
; paged external ram data
;--------------------------------------------------------
	rseg R_PSEG
;--------------------------------------------------------
; external ram data
;--------------------------------------------------------
	rseg R_XSEG
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	XSEG
;--------------------------------------------------------
; external initialized ram data
;--------------------------------------------------------
	rseg R_IXSEG
	rseg R_HOME
	rseg R_GSINIT
	rseg R_CSEG
;--------------------------------------------------------
; Reset entry point and interrupt vectors
;--------------------------------------------------------
	CSEG at 0x0000
	ljmp	_crt0
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	rseg R_HOME
	rseg R_GSINIT
	rseg R_GSINIT
;--------------------------------------------------------
; data variables initialization
;--------------------------------------------------------
	rseg R_DINIT
	; The linker places a 'ret' at the end of segment R_DINIT.
;--------------------------------------------------------
; code
;--------------------------------------------------------
	rseg R_CSEG
;------------------------------------------------------------
;Allocation info for local variables in function '_c51_external_startup'
;------------------------------------------------------------
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:16: char _c51_external_startup (void)
;	-----------------------------------------
;	 function _c51_external_startup
;	-----------------------------------------
__c51_external_startup:
	using	0
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:19: SFRPAGE = 0x00;
	mov	_SFRPAGE,#0x00
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:20: WDTCN = 0xDE; //First key
	mov	_WDTCN,#0xDE
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:21: WDTCN = 0xAD; //Second key
	mov	_WDTCN,#0xAD
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:28: SFRPAGE = 0x10;
	mov	_SFRPAGE,#0x10
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:29: PFE0CN  = 0x20; // SYSCLK < 75 MHz.
	mov	_PFE0CN,#0x20
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:30: SFRPAGE = 0x00;
	mov	_SFRPAGE,#0x00
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:51: CLKSEL = 0x00;
	mov	_CLKSEL,#0x00
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:52: CLKSEL = 0x00;
	mov	_CLKSEL,#0x00
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:53: while ((CLKSEL & 0x80) == 0);
L002001?:
	mov	a,_CLKSEL
	jnb	acc.7,L002001?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:54: CLKSEL = 0x03;
	mov	_CLKSEL,#0x03
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:55: CLKSEL = 0x03;
	mov	_CLKSEL,#0x03
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:56: while ((CLKSEL & 0x80) == 0);
L002004?:
	mov	a,_CLKSEL
	jnb	acc.7,L002004?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:61: P0MDOUT |= 0x10; // Enable UART0 TX as push-pull output
	orl	_P0MDOUT,#0x10
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:62: XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
	mov	_XBR0,#0x01
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:63: XBR1     = 0X00;
	mov	_XBR1,#0x00
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:64: XBR2     = 0x40; // Enable crossbar and weak pull-ups
	mov	_XBR2,#0x40
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:70: SCON0 = 0x10;
	mov	_SCON0,#0x10
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:71: CKCON0 |= 0b_0000_0000 ; // Timer 1 uses the system clock divided by 12.
	mov	_CKCON0,_CKCON0
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:72: TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
	mov	_TH1,#0xE6
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:73: TL1 = TH1;      // Init Timer1
	mov	_TL1,_TH1
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:74: TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
	anl	_TMOD,#0x0F
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:75: TMOD |=  0x20;                       
	orl	_TMOD,#0x20
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:76: TR1 = 1; // START Timer1
	setb	_TR1
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:77: TI = 1;  // Indicate TX0 ready
	setb	_TI
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:79: return 0;
	mov	dpl,#0x00
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'Timer3us'
;------------------------------------------------------------
;us                        Allocated to registers r2 
;i                         Allocated to registers r3 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:84: void Timer3us(unsigned char us)
;	-----------------------------------------
;	 function Timer3us
;	-----------------------------------------
_Timer3us:
	mov	r2,dpl
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:89: CKCON0|=0b_0100_0000;
	orl	_CKCON0,#0x40
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:91: TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	mov	_TMR3RL,#0xB8
	mov	(_TMR3RL >> 8),#0xFF
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:92: TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	mov	_TMR3,_TMR3RL
	mov	(_TMR3 >> 8),(_TMR3RL >> 8)
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:94: TMR3CN0 = 0x04;                 // Sart Timer3 and clear overflow flag
	mov	_TMR3CN0,#0x04
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:95: for (i = 0; i < us; i++)       // Count <us> overflows
	mov	r3,#0x00
L003004?:
	clr	c
	mov	a,r3
	subb	a,r2
	jnc	L003007?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:97: while (!(TMR3CN0 & 0x80));  // Wait for overflow
L003001?:
	mov	a,_TMR3CN0
	jnb	acc.7,L003001?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:98: TMR3CN0 &= ~(0x80);         // Clear overflow indicator
	anl	_TMR3CN0,#0x7F
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:95: for (i = 0; i < us; i++)       // Count <us> overflows
	inc	r3
	sjmp	L003004?
L003007?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:100: TMR3CN0 = 0 ;                   // Stop Timer3 and clear overflow flag
	mov	_TMR3CN0,#0x00
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'waitms'
;------------------------------------------------------------
;ms                        Allocated to registers r2 r3 
;j                         Allocated to registers r4 r5 
;k                         Allocated to registers r6 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:103: void waitms (unsigned int ms)
;	-----------------------------------------
;	 function waitms
;	-----------------------------------------
_waitms:
	mov	r2,dpl
	mov	r3,dph
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:107: for(j=0; j<ms; j++)
	mov	r4,#0x00
	mov	r5,#0x00
L004005?:
	clr	c
	mov	a,r4
	subb	a,r2
	mov	a,r5
	subb	a,r3
	jnc	L004009?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:108: for (k=0; k<4; k++) Timer3us(250);
	mov	r6,#0x00
L004001?:
	cjne	r6,#0x04,L004018?
L004018?:
	jnc	L004007?
	mov	dpl,#0xFA
	push	ar2
	push	ar3
	push	ar4
	push	ar5
	push	ar6
	lcall	_Timer3us
	pop	ar6
	pop	ar5
	pop	ar4
	pop	ar3
	pop	ar2
	inc	r6
	sjmp	L004001?
L004007?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:107: for(j=0; j<ms; j++)
	inc	r4
	cjne	r4,#0x00,L004005?
	inc	r5
	sjmp	L004005?
L004009?:
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'LCD_pulse'
;------------------------------------------------------------
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:111: void LCD_pulse (void)
;	-----------------------------------------
;	 function LCD_pulse
;	-----------------------------------------
_LCD_pulse:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:113: LCD_E=1;
	setb	_P2_5
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:114: Timer3us(40);
	mov	dpl,#0x28
	lcall	_Timer3us
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:115: LCD_E=0;
	clr	_P2_5
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'LCD_byte'
;------------------------------------------------------------
;x                         Allocated to registers r2 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:118: void LCD_byte (unsigned char x)
;	-----------------------------------------
;	 function LCD_byte
;	-----------------------------------------
_LCD_byte:
	mov	r2,dpl
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:121: ACC=x; //Send high nible
	mov	_ACC,r2
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:122: LCD_D7=ACC_7;
	mov	c,_ACC_7
	mov	_P2_1,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:123: LCD_D6=ACC_6;
	mov	c,_ACC_6
	mov	_P2_2,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:124: LCD_D5=ACC_5;
	mov	c,_ACC_5
	mov	_P2_3,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:125: LCD_D4=ACC_4;
	mov	c,_ACC_4
	mov	_P2_4,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:126: LCD_pulse();
	push	ar2
	lcall	_LCD_pulse
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:127: Timer3us(40);
	mov	dpl,#0x28
	lcall	_Timer3us
	pop	ar2
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:128: ACC=x; //Send low nible
	mov	_ACC,r2
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:129: LCD_D7=ACC_3;
	mov	c,_ACC_3
	mov	_P2_1,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:130: LCD_D6=ACC_2;
	mov	c,_ACC_2
	mov	_P2_2,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:131: LCD_D5=ACC_1;
	mov	c,_ACC_1
	mov	_P2_3,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:132: LCD_D4=ACC_0;
	mov	c,_ACC_0
	mov	_P2_4,c
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:133: LCD_pulse();
	ljmp	_LCD_pulse
;------------------------------------------------------------
;Allocation info for local variables in function 'WriteData'
;------------------------------------------------------------
;x                         Allocated to registers r2 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:136: void WriteData (unsigned char x)
;	-----------------------------------------
;	 function WriteData
;	-----------------------------------------
_WriteData:
	mov	r2,dpl
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:138: LCD_RS=1;
	setb	_P2_6
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:139: LCD_byte(x);
	mov	dpl,r2
	lcall	_LCD_byte
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:140: waitms(2);
	mov	dptr,#0x0002
	ljmp	_waitms
;------------------------------------------------------------
;Allocation info for local variables in function 'WriteCommand'
;------------------------------------------------------------
;x                         Allocated to registers r2 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:143: void WriteCommand (unsigned char x)
;	-----------------------------------------
;	 function WriteCommand
;	-----------------------------------------
_WriteCommand:
	mov	r2,dpl
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:145: LCD_RS=0;
	clr	_P2_6
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:146: LCD_byte(x);
	mov	dpl,r2
	lcall	_LCD_byte
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:147: waitms(5);
	mov	dptr,#0x0005
	ljmp	_waitms
;------------------------------------------------------------
;Allocation info for local variables in function 'LCD_4BIT'
;------------------------------------------------------------
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:150: void LCD_4BIT (void)
;	-----------------------------------------
;	 function LCD_4BIT
;	-----------------------------------------
_LCD_4BIT:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:152: LCD_E=0; // Resting state of LCD's enable is zero
	clr	_P2_5
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:154: waitms(20);
	mov	dptr,#0x0014
	lcall	_waitms
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:156: WriteCommand(0x33);
	mov	dpl,#0x33
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:157: WriteCommand(0x33);
	mov	dpl,#0x33
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:158: WriteCommand(0x32); // Change to 4-bit mode
	mov	dpl,#0x32
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:161: WriteCommand(0x28);
	mov	dpl,#0x28
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:162: WriteCommand(0x0c);
	mov	dpl,#0x0C
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:163: WriteCommand(0x01); // Clear screen command (takes some time)
	mov	dpl,#0x01
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:164: waitms(20); // Wait for clear screen command to finsih.
	mov	dptr,#0x0014
	ljmp	_waitms
;------------------------------------------------------------
;Allocation info for local variables in function 'LCDprint'
;------------------------------------------------------------
;line                      Allocated with name '_LCDprint_PARM_2'
;string                    Allocated to registers r2 r3 r4 
;j                         Allocated to registers r5 r6 
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:167: void LCDprint(char * string, unsigned char line, bit clear)
;	-----------------------------------------
;	 function LCDprint
;	-----------------------------------------
_LCDprint:
	mov	r2,dpl
	mov	r3,dph
	mov	r4,b
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:171: WriteCommand(line==2?0xc0:0x80);
	mov	a,#0x02
	cjne	a,_LCDprint_PARM_2,L010013?
	mov	r5,#0xC0
	sjmp	L010014?
L010013?:
	mov	r5,#0x80
L010014?:
	mov	dpl,r5
	push	ar2
	push	ar3
	push	ar4
	lcall	_WriteCommand
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:172: waitms(5);
	mov	dptr,#0x0005
	lcall	_waitms
	pop	ar4
	pop	ar3
	pop	ar2
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:173: for(j=0; string[j]!=0; j++)	WriteData(string[j]);// Write the message
	mov	r5,#0x00
	mov	r6,#0x00
L010003?:
	mov	a,r5
	add	a,r2
	mov	r7,a
	mov	a,r6
	addc	a,r3
	mov	r0,a
	mov	ar1,r4
	mov	dpl,r7
	mov	dph,r0
	mov	b,r1
	lcall	__gptrget
	mov	r7,a
	jz	L010006?
	mov	dpl,r7
	push	ar2
	push	ar3
	push	ar4
	push	ar5
	push	ar6
	lcall	_WriteData
	pop	ar6
	pop	ar5
	pop	ar4
	pop	ar3
	pop	ar2
	inc	r5
	cjne	r5,#0x00,L010003?
	inc	r6
	sjmp	L010003?
L010006?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:174: if(clear) for(; j<CHARS_PER_LINE; j++) WriteData(' '); // Clear the rest of the line
	jnb	_LCDprint_PARM_3,L010011?
	mov	ar2,r5
	mov	ar3,r6
L010007?:
	clr	c
	mov	a,r2
	subb	a,#0x10
	mov	a,r3
	xrl	a,#0x80
	subb	a,#0x80
	jnc	L010011?
	mov	dpl,#0x20
	push	ar2
	push	ar3
	lcall	_WriteData
	pop	ar3
	pop	ar2
	inc	r2
	cjne	r2,#0x00,L010007?
	inc	r3
	sjmp	L010007?
L010011?:
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'getsn'
;------------------------------------------------------------
;len                       Allocated with name '_getsn_PARM_2'
;buff                      Allocated with name '_getsn_buff_1_42'
;j                         Allocated with name '_getsn_sloc0_1_0'
;c                         Allocated to registers r3 
;sloc0                     Allocated with name '_getsn_sloc0_1_0'
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:177: int getsn (char * buff, int len)
;	-----------------------------------------
;	 function getsn
;	-----------------------------------------
_getsn:
	mov	_getsn_buff_1_42,dpl
	mov	(_getsn_buff_1_42 + 1),dph
	mov	(_getsn_buff_1_42 + 2),b
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:182: for(j=0; j<(len-1); j++)
	clr	a
	mov	_getsn_sloc0_1_0,a
	mov	(_getsn_sloc0_1_0 + 1),a
	mov	a,_getsn_PARM_2
	add	a,#0xff
	mov	r7,a
	mov	a,(_getsn_PARM_2 + 1)
	addc	a,#0xff
	mov	r0,a
	mov	r1,#0x00
	mov	r2,#0x00
L011005?:
	clr	c
	mov	a,r1
	subb	a,r7
	mov	a,r2
	xrl	a,#0x80
	mov	b,r0
	xrl	b,#0x80
	subb	a,b
	jnc	L011008?
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:184: c=getchar();
	push	ar2
	push	ar7
	push	ar0
	push	ar1
	lcall	_getchar
	mov	r3,dpl
	pop	ar1
	pop	ar0
	pop	ar7
	pop	ar2
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:185: if ( (c=='\n') || (c=='\r') )
	cjne	r3,#0x0A,L011015?
	sjmp	L011001?
L011015?:
	cjne	r3,#0x0D,L011002?
L011001?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:187: buff[j]=0;
	mov	a,_getsn_sloc0_1_0
	add	a,_getsn_buff_1_42
	mov	r4,a
	mov	a,(_getsn_sloc0_1_0 + 1)
	addc	a,(_getsn_buff_1_42 + 1)
	mov	r5,a
	mov	r6,(_getsn_buff_1_42 + 2)
	mov	dpl,r4
	mov	dph,r5
	mov	b,r6
	clr	a
	lcall	__gptrput
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:188: return j;
	mov	dpl,_getsn_sloc0_1_0
	mov	dph,(_getsn_sloc0_1_0 + 1)
	ret
L011002?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:192: buff[j]=c;
	mov	a,r1
	add	a,_getsn_buff_1_42
	mov	r4,a
	mov	a,r2
	addc	a,(_getsn_buff_1_42 + 1)
	mov	r5,a
	mov	r6,(_getsn_buff_1_42 + 2)
	mov	dpl,r4
	mov	dph,r5
	mov	b,r6
	mov	a,r3
	lcall	__gptrput
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:182: for(j=0; j<(len-1); j++)
	inc	r1
	cjne	r1,#0x00,L011018?
	inc	r2
L011018?:
	mov	_getsn_sloc0_1_0,r1
	mov	(_getsn_sloc0_1_0 + 1),r2
	sjmp	L011005?
L011008?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:195: buff[j]=0;
	mov	a,_getsn_sloc0_1_0
	add	a,_getsn_buff_1_42
	mov	r2,a
	mov	a,(_getsn_sloc0_1_0 + 1)
	addc	a,(_getsn_buff_1_42 + 1)
	mov	r3,a
	mov	r4,(_getsn_buff_1_42 + 2)
	mov	dpl,r2
	mov	dph,r3
	mov	b,r4
	clr	a
	lcall	__gptrput
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:196: return len;
	mov	dpl,_getsn_PARM_2
	mov	dph,(_getsn_PARM_2 + 1)
	ret
;------------------------------------------------------------
;Allocation info for local variables in function 'main'
;------------------------------------------------------------
;buff                      Allocated with name '_main_buff_1_48'
;------------------------------------------------------------
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:199: void main (void)
;	-----------------------------------------
;	 function main
;	-----------------------------------------
_main:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:203: LCD_4BIT();
	lcall	_LCD_4BIT
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:206: LCDprint("LCD 4-bit test:", 1, 1);
	mov	_LCDprint_PARM_2,#0x01
	setb	_LCDprint_PARM_3
	mov	dptr,#__str_0
	mov	b,#0x80
	lcall	_LCDprint
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:207: LCDprint("Hello, World!", 2, 1);
	mov	_LCDprint_PARM_2,#0x02
	setb	_LCDprint_PARM_3
	mov	dptr,#__str_1
	mov	b,#0x80
	lcall	_LCDprint
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:208: while(1)
L012002?:
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:210: printf("Type what you want to display in line 2 (16 char max): ");
	mov	a,#__str_2
	push	acc
	mov	a,#(__str_2 >> 8)
	push	acc
	mov	a,#0x80
	push	acc
	lcall	_printf
	dec	sp
	dec	sp
	dec	sp
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:211: getsn(buff, sizeof(buff));
	mov	_getsn_PARM_2,#0x11
	clr	a
	mov	(_getsn_PARM_2 + 1),a
	mov	dptr,#_main_buff_1_48
	mov	b,#0x40
	lcall	_getsn
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:212: printf("\n");
	mov	a,#__str_3
	push	acc
	mov	a,#(__str_3 >> 8)
	push	acc
	mov	a,#0x80
	push	acc
	lcall	_printf
	dec	sp
	dec	sp
	dec	sp
;	C:\ELEC291\Elec-291\lab4-joe-marcus\EFM8_LCD_4bit.c:213: LCDprint(buff, 2, 1);
	mov	_LCDprint_PARM_2,#0x02
	setb	_LCDprint_PARM_3
	mov	dptr,#_main_buff_1_48
	mov	b,#0x40
	lcall	_LCDprint
	sjmp	L012002?
	rseg R_CSEG

	rseg R_XINIT

	rseg R_CONST
__str_0:
	db 'LCD 4-bit test:'
	db 0x00
__str_1:
	db 'Hello, World!'
	db 0x00
__str_2:
	db 'Type what you want to display in line 2 (16 char max): '
	db 0x00
__str_3:
	db 0x0A
	db 0x00

	CSEG

end
