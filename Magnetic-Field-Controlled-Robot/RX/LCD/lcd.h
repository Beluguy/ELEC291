#define F_CPU 32000000L

#define LCD_RS_0 (GPIOA->ODR &= ~BIT0)
#define LCD_RS_1 (GPIOA->ODR |= BIT0)
#define LCD_E_0  (GPIOA->ODR &= ~BIT1)
#define LCD_E_1  (GPIOA->ODR |= BIT1)
#define LCD_D4_0 (GPIOA->ODR &= ~BIT2)
#define LCD_D4_1 (GPIOA->ODR |= BIT2)
#define LCD_D5_0 (GPIOA->ODR &= ~BIT3)
#define LCD_D5_1 (GPIOA->ODR |= BIT3)
#define LCD_D6_0 (GPIOA->ODR &= ~BIT4)
#define LCD_D6_1 (GPIOA->ODR |= BIT4)
#define LCD_D7_0 (GPIOA->ODR &= ~BIT5)
#define LCD_D7_1 (GPIOA->ODR |= BIT5)
#define CHARS_PER_LINE 16

void Delay_us(unsigned char us);
void waitms (unsigned int ms);
void LCD_pulse (void);
void LCD_byte (unsigned char x);
void WriteData (unsigned char x);
void WriteCommand (unsigned char x);
void LCD_4BIT (void);
void LCDprint(char * string, unsigned char line, unsigned char clear);
