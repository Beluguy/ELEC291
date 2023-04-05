#define LCD_RS P3_0
//#define LCD_RW Px_x // Not used in this code.  Connect to GND
#define LCD_E  P2_6
#define LCD_D4 P2_5
#define LCD_D5 P2_4
#define LCD_D6 P2_3
#define LCD_D7 P2_2
#define CHARS_PER_LINE 16

void Timer3us(unsigned char us);
void waitms (unsigned int ms);
void LCD_pulse (void);
void LCD_byte (unsigned char x);
void WriteData (unsigned char x);
void WriteCommand (unsigned char x);
void LCD_4BIT (void);
void LCDprint(char * string, unsigned char line, bit clear);
