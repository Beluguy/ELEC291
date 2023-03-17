#include "../Include/stm32l051xx.h"
#ifdef USE_USART1
    #include "../Include/serial.h"
#endif

// Allocated stack space as defined in linker script
extern unsigned int _StackTop;

void ResetInit(void);
void Hard_Fault_Handler(void);
void Default_Handler(void); /* Default empty handler */
void main(void);

/* Cortex-M0+ core interrupt handlers */
void NMI_Handler           ( void ) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler           ( void ) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler        ( void ) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler       ( void ) __attribute__ ((weak, alias("Default_Handler")));

/* Peripherals interrupt handlers */
void WWDG_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void PVD_Handler           ( void ) __attribute__ ((weak, alias("Default_Handler")));
void RTC_Handler           ( void ) __attribute__ ((weak, alias("Default_Handler")));
void FLASH_Handler         ( void ) __attribute__ ((weak, alias("Default_Handler")));
void RCC_Handler           ( void ) __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_1_Handler       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_3_Handler       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_15_Handler      ( void ) __attribute__ ((weak, alias("Default_Handler")));
void DMA_CH1_Handler       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void DMA_CH2_3_Handler     ( void ) __attribute__ ((weak, alias("Default_Handler")));
void DMA_CH4_5_6_7_Handler ( void ) __attribute__ ((weak, alias("Default_Handler")));
void ADC_COMP_Handler      ( void ) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM1_Handler        ( void ) __attribute__ ((weak, alias("Default_Handler")));
void TIM2_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void TIM6_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void TIM21_Handler         ( void ) __attribute__ ((weak, alias("Default_Handler")));
void TIM22_Handler         ( void ) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_Handler          ( void ) __attribute__ ((weak, alias("Default_Handler")));
void USART1_Handler        ( void ) __attribute__ ((weak, alias("Default_Handler")));
void USART2_Handler        ( void ) __attribute__ ((weak, alias("Default_Handler")));
void LPUART1_Handler       ( void ) __attribute__ ((weak, alias("Default_Handler")));

// The section "vectors" is placed at the beginning of flash by the linker script stm32l05xxx.ld
const void * Vectors[] __attribute__((section(".vectors"))) ={
	&_StackTop,            /* Top of stack */
	ResetInit,   	       /* -15: Reset Handler */
	NMI_Handler,           /* -14: NMI */
	Hard_Fault_Handler,    /* -13: Hard Fault.  If not using the right libraries this will happen. */
	0,                     /* -12: Reserved */
	0,                     /* -11: Reserved */
	0,                     /* -10: Reserved */
	0,                     /* -9:  Reserved */
	0,                     /* -8:  Reserved */
	0,                     /* -7:  Reserved */
	0,                     /* -6:  Reserved */
	SVC_Handler,           /* -5:  SVC */
	0,                     /* -4:  Reserved */
	0,                     /* -3:  Reserved */
	PendSV_Handler,        /* -2:  PendSV */
	SysTick_Handler,       /* -1:  SysTick */	
    /* External interrupt handlers follow */
	WWDG_Handler,          /* 0:  WWDG */
	PVD_Handler,           /* 1:  PVD */
	RTC_Handler,           /* 2:  RTC */
	FLASH_Handler,         /* 3:  FLASH */
	RCC_Handler,           /* 4:  RCC */
	EXTI0_1_Handler,       /* 5:  EXTI0_1 */
	EXTI2_3_Handler,       /* 6:  EXTI2_3 */
	EXTI4_15_Handler,      /* 7:  EXTI4_15 */
	0,                     /* 8:  Reserved */
	DMA_CH1_Handler,       /* 9:  DMA_CH1 */
	DMA_CH2_3_Handler,     /* 10: DMA_CH2_3 */
	DMA_CH4_5_6_7_Handler, /* 11: DMA_CH4_5_6_7 */
	ADC_COMP_Handler,      /* 12: ADC_COMP */
	LPTIM1_Handler,        /* 13: LPTIM1 */
	0,                     /* 14: Reserved */
	TIM2_Handler,          /* 15: TIM2 */
	0,                     /* 16: Reserved */
	TIM6_Handler,          /* 17: TIM6 */
	0,                     /* 18: Reserved */
	0,                     /* 19: Reserved */
	TIM21_Handler,         /* 20: TIM21 */
	0,                     /* 21: Reserved */
	TIM22_Handler,         /* 22: TIM22 */
	I2C1_Handler,          /* 23: I2C1 */
	I2C2_Handler,          /* 24: I2C2 */
	SPI1_Handler,          /* 25: SPI1 */
	SPI2_Handler,          /* 26: SPI2 */
	USART1_Handler,        /* 27: USART1 */
	USART2_Handler,        /* 28: USART2 */
	LPUART1_Handler        /* 29: LPUART1 */
};

// gcc and g++ compatible C runtime initialization.
extern unsigned int _etext, _data, _edata, _bss, _ebss;
extern unsigned int __init_array_start;
extern unsigned int __init_array_end;
//
static inline void crt0 (void)
{
    unsigned int *src, *dest;

    // copy the data section
    src  = &_etext;
    dest = &_data;
    while (dest < &_edata)
        *(dest++) = *(src++);

    // blank the bss section
    while (dest < &_ebss)
        *(dest++) = 0;

    // call C++ constructors
    dest = &__init_array_start;
    while (dest < &__init_array_end)
      (*(void(**)(void)) dest++)();
}

// https://forum.digikey.com/t/getting-started-with-stm32-arm-cortex-m0/13328
void initClock(void)
{
	// STEP1: Enable HSI16
	RCC->CR |= ((uint32_t)RCC_CR_HSION);   
	// Wait for HSI to be ready
	while ((RCC->CR & RCC_CR_HSIRDY) == 0)
	{
	}
	
	// STEP2: Set HSI to System Clock
	RCC->CFGR = RCC_CFGR_SW_HSI;   
	// Wait for HSI to be used for the system clock
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
	{
	}
	
	// STEP3: Setup FLASH and PWR
	FLASH->ACR |= FLASH_ACR_PRFTEN;        // Enable Prefetch Buffer
	FLASH->ACR |= FLASH_ACR_LATENCY;       // Flash 1 wait state
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;     // Enable the PWR APB1 Clock
	PWR->CR = PWR_CR_VOS_0;                // Select the Voltage Range 1 (1.8V)
	while((PWR->CSR & PWR_CSR_VOSF) != 0); // Wait for Voltage Regulator Ready
	
	// STEP4: PLLCLK = (HSI * 4)/2 = 32 MHz
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV); // Clear
	RCC->CFGR |=  (RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL4 | RCC_CFGR_PLLDIV2);
	
	// STEP5: Setup Peripheral Clock Divisors
	// We can leave all the peripherals to have a Divisor of 1.
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // HCLK = SYSCLK
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV1; // PCLK1 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // PCLK2 = HCLK
	
	// STEP6: Set PLL as System Clock
	RCC->CR &= ~RCC_CR_PLLON; // Disable PLL
	RCC->CR |= RCC_CR_PLLON;  // Enable PLL
    
    // Wait until the PLL is ready
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
	{
	}
    
	// Select PLL as system Clock
	RCC->CFGR &= ~RCC_CFGR_SW;        // Clear
	RCC->CFGR |=  RCC_CFGR_SW_PLL;    // Set
    
	// Wait for PLL to become system core clock
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
	{
	}
}

// Reset entry point. Sets up the C runtime environment.
__attribute__ ((section(".after_vectors"), naked))
void ResetInit()
{
	initClock(); // Set clock to 32 MHz
    crt0();
#ifdef USE_USART1
	initUART(115200);
	__enable_irq();

	//eputs("Going to main()\r\n");

	#ifdef SHOW_LD_SCRIPT_ASSIGMENTS
	PRINTVAR(&_etext)
	PRINTVAR(&_data)	    
	PRINTVAR(&_edata)	    
	PRINTVAR(&_bss)	    
	PRINTVAR(&_ebss)
	PRINTVAR(&__init_array_start)
	PRINTVAR(&__init_array_end)	    
	#endif
#endif
   
	main();
#ifdef USE_USART1
	eputs("\r\nReturned from main().  System stop.");
#endif
	while (1) ; // hang if main returns
}

void Default_Handler()
{
#ifdef USE_USART1
	eputs("Interrupt without handler (ISR).  System stop.");
#endif
	while(1);
}

void Hard_Fault_Handler()
{
#ifdef USE_USART1
	eputs("Hard Fault interrupt.  System stop.");
#endif
	while(1);
}