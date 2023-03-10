		.thumb /*Cortex micros only understand thumb(2) code	*/
		.global Vector_Table, ResetVector
		.global USART1_IRQHandler	
/* Set up interrupt vector table */
		.text
Vector_Table:    
		.word     0x20002000          /* Top of Stack */
/* The interrupt vectors that follow are manually forced to be aligned along
   odd addresses.  The reason for this is that the address for a Thumb instruction
   must always have its LSB set to 1.  This does not mean that the instruction 
   is stored at an odd number address.  The LSB is used as a flag to indicate
   that an ARM (LSB=0) or a Thumb (LSB=1) instruction is being addressed. */
ResetVector:    
	.word     Reset_Handler+1     /* -15: Reset Handler */
	.word     Default_Handler+1   /* -14: NMI */
	.word     Default_Handler+1   /* -13: Hard Fault */
	.word     0                   /* -12: Reserved */
	.word     0                   /* -11: Reserved */
	.word     0                   /* -10: Reserved */
	.word     0                   /* -9:  Reserved */
	.word     0                   /* -8:  Reserved */
	.word     0                   /* -7:  Reserved */
	.word     0                   /* -6:  Reserved */
	.word     Default_Handler+1   /* -5:  SVC */
	.word     0                   /* -4:  Reserved */
	.word     0                   /* -3:  Reserved */
	.word     Default_Handler+1   /* -2:  PendSV */
	.word     Default_Handler+1   /* -1:  SysTick */	
/* External interrupt handlers follow */
	.word	  Default_Handler + 1 /* 0:  WWDG */
	.word	  Default_Handler + 1 /* 1:  PVD */
	.word	  Default_Handler + 1 /* 2:  RTC */
	.word	  Default_Handler + 1 /* 3:  FLASH */
	.word	  Default_Handler + 1 /* 4:  RCC */
	.word	  Default_Handler + 1 /* 5:  EXTI0_1 */
	.word	  Default_Handler + 1 /* 6:  EXTI2_3 */
	.word	  Default_Handler + 1 /* 7:  EXTI4_15 */
	.word	  Default_Handler + 1 /* 8:  RESERVED */
	.word	  Default_Handler + 1 /* 9:  DMA_CH1 */
	.word	  Default_Handler + 1 /* 10: DMA_CH2_3 */
	.word	  Default_Handler + 1 /* 11: DMA_CH4_5_6_7 */
	.word	  Default_Handler + 1 /* 12: ADC_COMP */
	.word	  Default_Handler + 1 /* 13: LPTIM1 */
	.word	  Default_Handler + 1 /* 14: RESERVED */
	.word	  Default_Handler + 1 /* 15: TIM2 */
	.word	  Default_Handler + 1 /* 16: RESERVED */
	.word	  Default_Handler + 1 /* 17: TIM6 */
	.word	  Default_Handler + 1 /* 18: RESERVED */
	.word	  Default_Handler + 1 /* 19: RESERVED */
	.word	  Default_Handler + 1 /* 20: TIM21 */
	.word	  Default_Handler + 1 /* 21: RESERVED */
	.word	  Default_Handler + 1 /* 22: TIM22 */
	.word	  Default_Handler + 1 /* 23: I2C1 */
	.word	  Default_Handler + 1 /* 24: I2C2 */
	.word	  Default_Handler + 1 /* 25: SPI1 */
	.word	  Default_Handler + 1 /* 26: SPI2 */
	.word	  USART1_IRQHandler   /* 27: USART1 */
	.word	  Default_Handler + 1 /* 28: USART2 */
	.word	  Default_Handler + 1 /* 29: LPUART1 */

/* Reset handler */
		.thumb
Reset_Handler:
	.global Reset_Handler, Default_Handler
    .extern  main
/* Stack pointer is already set to the value at entry 0 in the interrupt vector table  */
/* Clock speed is set by default to the internal factory calibrated 8MHz RC oscillator */
/* The PLL is configured to run at twice the RC clock speed so the default system      */
/* operating speed is 16MHz							       */
/* All that is left to do is to call main					       */
		bl	main
		b	.	/* If main returns enter a loop */
Default_Handler:
		b       .	/* Default handler just enters a loop.  Note the dot */



