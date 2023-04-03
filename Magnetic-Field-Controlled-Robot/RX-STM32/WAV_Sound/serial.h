#define NEWLINE 0x0d
#define LINEFEED 0x0a

void initUART(int BaudRate);
unsigned char uart_getc(void);
void uart_putc(unsigned char c);
int eputs(char *String);
int egets(char *String, int size);
void eputc(char c);
char egetc(void);

#define PRINTVAR(x) eputs(#x": 0x"); printHex((int)x); eputs("\n\r");