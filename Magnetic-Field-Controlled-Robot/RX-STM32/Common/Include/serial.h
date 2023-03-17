#define NEWLINE 0x0d
#define LINEFEED 0x0a

void initUART(int BaudRate);
int ReadCom(int Max,unsigned char *Buffer);
int WriteCom(int Count,unsigned char *Buffer);
int eputs(char *String);
int egets(char *String, int size);
int egets_echo(char *String, int size);
void eputc(char c);
char egetc(void);
char egetc_echo(void);

#define PRINTVAR(x) eputs(#x": 0x"); printHex((int)x); eputs("\n\r");