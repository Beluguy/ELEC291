/////////////////////////////////////////////////////////////////////////////
//
// Computer_Sender.c:  SPI Serial flash loader.
// 
// Copyright (C) 2012-2021  Jesus Calvino-Fraga, jesusc (at) ece.ubc.ca
// 
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any
// later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
/////////////////////////////////////////////////////////////////////////////
//
// Compile using Visual C:
// cl Computer_Sender.c
//
// or compile using gcc for windows:
// gcc Computer_Sender.c -o Computer_Sender.exe
//
// For macOS and Linux:
// gcc Computer_Sender.c -o Computer_Sender
//

#ifdef __APPLE__
	#define __unix__ 1
#endif

#ifdef __unix__ 
	#ifdef __APPLE__
		#include <termios.h>
	#else
		#include <termio.h>
	#endif
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/signal.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <math.h>
	#include <time.h>
	#include <string.h>
	#include <errno.h>
	#include <ctype.h>
	#include <string.h>
	#include <stdbool.h>
	#include <limits.h>
	
	#define strnicmp strncasecmp 
	#define _strnicmp strncasecmp 
	#define stricmp strcasecmp
	#define _stricmp strcasecmp
	#define TRUE true
	#define FALSE false
	#define MAX_PATH PATH_MAX
	#define DWORD unsigned long int
	#define BOOL bool
	
	#define WriteFile(X1, X2, X3, X4, X5) *(X4)=write(fd, X2, X3);
	#define ReadFile(X1, X2, X3, X4, X5) *(X4)=read(fd, X2, X3);
	#define FlushFileBuffers(X1) {unsigned char jj; while(read(fd, &jj, 1));}

#else
	#include <windows.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
#endif

#define EQ(X,Y)  (_stricmp(X, Y)==0)
#define NEQ(X,Y) (_stricmp(X, Y)!=0)

clock_t startm, stopm;
#define START if ( (startm = clock()) == -1) {printf("Error calling clock");}
#define STOP if ( (stopm = clock()) == -1) {printf("Error calling clock");}
#define PRINTTIME printf( "%.1f seconds.", ((double)stopm-startm)/CLOCKS_PER_SEC);

#define ZERO_MAX (0x80+2)
#define ZERO_MIN (0x80-2)

char m_Serial[0x100]="";
int m_memsize=0;
int m_timeout=15;
int m_reset=0;
BOOL b_default_CBUS=FALSE;
int Selected_Device=-1;

char InName[MAX_PATH]="";
char OutNameAsm[MAX_PATH]="";
char OutNameC[MAX_PATH]="";
char OutNameRead[MAX_PATH]="";

#ifdef __unix__
int fd;
char SerialPort[MAX_PATH]="/dev/ttyUSB0";
struct termios comio;

int Select_Baud (int Baud_Rate)
{
	switch (Baud_Rate)
	{
		case 115200: return B115200;
		case 57600:  return B57600;
		case 38400:  return B38400;
		case 19200:  return B19200;
		case 9600:   return B9600;
		case 4800:   return B4800;
		case 2400:   return B2400;
		case 1800:   return B1800;
		case 1200:   return B1200;
		default:     return B4800;
	}
}

//CSIZE  Character size mask.  Values are CS5, CS6, CS7, or CS8.
//CSTOPB Set two stop bits, rather than one.
//PARENB Enable parity generation on output and parity checking for input.
//PARODD If set, then parity for input and output is odd; otherwise even parity is used.
#define ONESTOPBIT 0 
#define TWOSTOPBITS CSTOPB 
#define NOPARITY 0

int OpenSerialPort(char * devicename, int baud, int parity, int numbits, int numstop)
{
    struct termios options;
	speed_t BAUD;
	
	BAUD=Select_Baud(baud);
	
	//open the device(com port) to be non-blocking (read will return immediately)
	fd = open(devicename, O_RDWR | O_NOCTTY | O_NDELAY );
	if (fd < 0)
	{
		perror(devicename);
		return(1);
	}
	
	/*Reading data from a port is a little trickier. When you operate the port
	in raw data mode, each read system call will return however many characters
	are actually available in the serial input buffers. If no characters are
	available, the call will block (wait) until characters come in, an interval
	timer expires, or an error occurs. The read function can be made to return
	immediately by doing the following:*/	

	/* The FNDELAY option causes the read function to return 0 if no characters
	are available on the port. To restore normal (blocking) behavior, call
	fcntl() without the FNDELAY option:*/
	
 	//fcntl(fd, F_SETFL, FNDELAY);
    fcntl(fd, F_SETFL, 0);
	
	/*This is also used after opening a serial port with the O_NDELAY option.*/

	// Make the file descriptor asynchronous 
	
	tcgetattr(fd, &comio);
	
	//newtio.sg_ispeed = newtio.sg_ospeed = BAUD;
	cfsetospeed(&comio, (speed_t)BAUD);
	cfsetispeed(&comio, (speed_t)BAUD);
	
	comio.c_cflag = BAUD | CS8 | CSTOPB | CLOCAL | CREAD;
	comio.c_cflag &= ~(CRTSCTS); /* No hardware flow control */
	comio.c_iflag = IGNPAR;
	comio.c_iflag &= ~(IXON | IXOFF | IXANY); /* No software flow control */
	comio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Raw input mode*/
	comio.c_oflag &= ~OPOST; /* Raw ouput mode */
	comio.c_cc[VMIN]=0;
	comio.c_cc[VTIME]=1;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &comio);

	return(0);
}

void CloseSerialPort(void)
{
	close(fd);
}

void Sleep (int msec)
{
	struct timespec req;

	req.tv_sec=0;
	req.tv_nsec=1000000L*msec;
    nanosleep (&req, NULL);
}

void Delay (int usec)
{
	struct timespec req;

	req.tv_sec=0;
	req.tv_nsec=1000L*usec;
    nanosleep (&req, NULL);
}

#else // For Windows
HANDLE hComm=INVALID_HANDLE_VALUE;
char SerialPort[MAX_PATH]="COM1";

int OpenSerialPort (char * devicename, DWORD baud, BYTE parity, BYTE bits, BYTE stop)
{
	char sPort[16];
	DCB dcb;
	COMMTIMEOUTS Timeouts;

	sprintf(sPort, "\\\\.\\%s", devicename);
	
	hComm = CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	
	if (!GetCommState(hComm, &dcb))
	{
		printf("Failed in call to GetCommState()\n");
		fflush(stdout);
		return -1;
	}

	dcb.fAbortOnError=FALSE;
	dcb.BaudRate = baud; 
	dcb.Parity = parity;
	dcb.ByteSize = bits;
	dcb.StopBits = stop;
	dcb.fDsrSensitivity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;

	//Now that we have all the settings in place, make the changes
	if (!SetCommState(hComm, &dcb))
	{
		printf("Failed in call to SetCommState()\n");
		fflush(stdout);
		return -1;
	}	

	ZeroMemory(&Timeouts, sizeof(COMMTIMEOUTS));
	Timeouts.ReadIntervalTimeout = 250;
	Timeouts.ReadTotalTimeoutMultiplier = 10;
	Timeouts.ReadTotalTimeoutConstant = 250;
	Timeouts.WriteTotalTimeoutMultiplier = 10;
	Timeouts.WriteTotalTimeoutConstant = 250;
	if (!SetCommTimeouts(hComm, &Timeouts))
	{
		printf("Failed in call to SetCommTimeouts\n");
		fflush(stdout);
		return -1;
	}

	return 0;
}

int CloseSerialPort (void)
{
	BOOL bSuccess;
	bSuccess = CloseHandle(hComm);
    hComm = INVALID_HANDLE_VALUE;
    if (!bSuccess)
    {
		printf("Failed to close serial port, GetLastError:%d\n", GetLastError());
		fflush(stdout);
		return -1;
	}
	return 0;
}

#endif

long fsize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
}

int Write_Flash(int address, unsigned char * buff, int len) 
{
	DWORD j;
	unsigned char bufftx[0x200];
	unsigned char buffrx[0x10];
	unsigned char empty=1, same=1;
	int count;

	if(len == 256)
	{
		for (j=1; j<len; j++)
		{
			if (buff[j]!=buff[0]) same=0;
		}
	}
	else
	{
		same=0;
	}

	if(same==0)
	{
		bufftx[0]='#';
		bufftx[1]='2';
		bufftx[2]=(address>>16) & 0xff;
		bufftx[3]=(address>>8)  & 0xff;
		bufftx[4]=(address>>0)  & 0xff;
		bufftx[5]=len & 0xff; // 0 means 256 bytes
	
		for (j=0; j<len; j++)
		{
			bufftx[6+j]=buff[j];
			if (buff[j]!=0xff) empty=0; // Don't send empty lines
		}
	
		if(empty==0)
		{	
			WriteFile(hComm, bufftx, len+6, &j, NULL);
			
			count=0;
			do {
				Sleep(1);
				ReadFile(hComm, buffrx, 1, &j, NULL);
				count++;
			} while ( (j==0) && (count<10) );		
			
			return j;
		}
	}
	else // All the bytes are the same. Use fill flash page command
	{
		bufftx[0]='#';
		bufftx[1]='6';
		bufftx[2]=(address>>16) & 0xff;
		bufftx[3]=(address>>8)  & 0xff;
		bufftx[4]=(address>>0)  & 0xff;
		bufftx[5]=buff[0]; // value to fill page with

		WriteFile(hComm, bufftx, 6, &j, NULL);
		
		count=0;
		do {
			Sleep(1);
			ReadFile(hComm, buffrx, 1, &j, NULL);
			count++;
		} while ( (j==0) && (count<10) );		
		
		return j;
	}
	
	return 0;
}

int Read_Flash(int address, unsigned char * buff, int len) 
{
	DWORD j;
	unsigned char bufftx[0x10];
	unsigned char empty=1;

	bufftx[0]='#';
	bufftx[1]='3';
	bufftx[2]=(address>>16) & 0xff;
	bufftx[3]=(address>>8)  & 0xff;
	bufftx[4]=(address>>0)  & 0xff;
	bufftx[5]=len & 0xff; // 0 means 256 bytes

	WriteFile(hComm, bufftx, 6, &j, NULL);
	ReadFile(hComm, buff, len, &j, NULL);
	
	return j;
}

unsigned char Erase_Flash(void) 
{
	DWORD j;
	unsigned char bufftx[0x10];
	unsigned char buffrx[0x10];
	int count=0;

	bufftx[0]='#';
	bufftx[1]='1';
	WriteFile(hComm, bufftx, 2, &j, NULL);
	
	count=0;
	do {
		Sleep(100);
		ReadFile(hComm, buffrx, 1, &j, NULL);
		count++;
		if(buffrx[0]==0x01) return 0x01;
	} while ( (j==0) && (count<100) );	
		
	return 0;
}

BOOL Identify (void)
{
	DWORD j;
    int i;
	unsigned char x[3];
    unsigned char c[4];

	FlushFileBuffers(hComm);
	x[0]='#';
	x[1]='0';
	WriteFile(hComm, x, 2, &j, NULL);
	Sleep(1);
	
	j=0;
	ReadFile(hComm, c, 3, &j, NULL);	
    if(j==3)
	{
		printf("Manufacturer: 0x%02x, Type: 0x%02x, Size: 0x%02x\n", c[0], c[1], c[2]); fflush(stdout);
		m_memsize=(1<<c[2]);
		printf("Memory has %d bytes\n", m_memsize); fflush(stdout);
		return TRUE;
	}
	else
	{
		printf("x");
		fflush(stdout);
	}

	return FALSE;
}

void play_stored (int sound_start, int sound_length)
{
	DWORD j;
	unsigned char bufftx[0x10];
	unsigned char empty=1;
	    
	FlushFileBuffers(hComm);

	bufftx[0]='#';
	bufftx[1]='4';
	bufftx[2]=(sound_start>>16) & 0xff;
	bufftx[3]=(sound_start>>8)  & 0xff;
	bufftx[4]=(sound_start>>0)  & 0xff;
	bufftx[5]=(sound_length>>16) & 0xff;
	bufftx[6]=(sound_length>>8)  & 0xff;
	bufftx[7]=(sound_length>>0)  & 0xff;

	WriteFile(hComm, bufftx, 8, &j, NULL);
}

void Flash(unsigned char * wavbuff, int wavsize) 
{
	unsigned char buff[0x100];
	unsigned char buffrx[0x10];
	int i, j, k, n;
	int count;
  
	START; // Measure the time it takes to program the microcontroller
	
	printf("Erasing flash memory...\n"); fflush(stdout);
	if(Erase_Flash()!=0x01)
	{
		printf("\nERROR: Flash erase command failed.\n");
		fflush(stdout);
		goto The_end;
	}

	printf(" Done.\nWriting flash memory...\n"); fflush(stdout);
	for(j=0, k=0; j<wavsize; j+=256)
    {
    	if((wavsize-j)<256)
    	{
    		n=wavsize-j;
    	}
    	else
    	{
    		n=256;
    	}
		Write_Flash(j, &wavbuff[j], n);
    	printf(".");
		if(++k==64)
		{
			k=0;
    		printf("\n");
		}
		fflush(stdout);	
	}
	
    printf(" Done.\n");
	
    printf("Actions completed in ");
    STOP;
	PRINTTIME;
	printf("\n");
	    
The_end:
	fflush(stdout);

}

unsigned int get_crc16 (int length)
{
	DWORD j;
	double maxwait, elapsed;
	unsigned char bufftx[0x10];
	unsigned char buffrx[0x10];
    clock_t start;
	    
	FlushFileBuffers(hComm);

	bufftx[0]='#';
	bufftx[1]='5';
	bufftx[2]=(length>>16) & 0xff;
	bufftx[3]=(length>>8)  & 0xff;
	bufftx[4]=(length>>0)  & 0xff;

	WriteFile(hComm, bufftx, 5, &j, NULL);
	
	// Assume it takes the microcontroller about 1 second for every 30k bytes of crc16 calculation
	maxwait=length/30000.0;
	maxwait+=0.5; // Some extra time just in case
	if(maxwait<1.0) maxwait=1.0;
	start = clock();
	
	j=0;
	do
	{
		buffrx[0]=0;
		buffrx[1]=0;
		ReadFile(hComm, buffrx, 2, &j, NULL); // Timeout is 200 ms?
		printf("."); fflush(stdout);
		elapsed=(double)(clock() - start)/CLOCKS_PER_SEC;
	} while ( (j!=2) && (elapsed<maxwait) );
	
	printf("\n"); fflush(stdout);
    
    return buffrx[0]*0x100+buffrx[1];
}

int Check_Wav (FILE * fp)
{
	char c[5];
	int nbRead;
	int chunkSize;
	int subChunk1Size;
	int subChunk2Size;
	short int audFormat;
	short int nbChannels;
	int sampleRate;
	int byteRate;
	short int blockAlign;
	short int bitsPerSample;

	fseek(fp,0,SEEK_SET);
	
	c[4] = 0;

	nbRead=fread(c, sizeof(char), 4, fp);
	if (nbRead < 4) return 0; // EOF?

	if (strcmp(c, "RIFF") != 0) return 0; // Not a RIFF?

	nbRead=fread(&chunkSize, sizeof(int), 1, fp);
	if (nbRead < 1) return 0; // EOF?

	nbRead=fread(c, sizeof(char), 4, fp);
	if (nbRead < 4) return 0; // EOF?

	if (strcmp(c, "WAVE") != 0) return 0; // Not a WAVE?

	nbRead=fread(c, sizeof(char), 4, fp);
	if (nbRead < 4) return 0;

	// Not a "fmt " subchunk?
	if (strcmp(c, "fmt ") != 0) return 0;
	
	nbRead=fread(&subChunk1Size, sizeof(int), 1, fp); // read size of chunk.
	if (nbRead < 1) return 0; // EOF?

	nbRead=fread(&audFormat, sizeof(short int), 1, fp);
	if (nbRead < 1) return 0; // EOF?

	if (audFormat != 1) return 0; // is it PCM?

	nbRead=fread(&nbChannels, sizeof(short int), 1, fp);
	if (nbRead < 1) return 0; // EOF?

	if (nbChannels != 1)
	{
		printf("Only WAV files with one audio channel supported.\n");
		return 0;
	}

	nbRead=fread(&sampleRate, sizeof(int), 1, fp);
	if (nbRead < 1) return 0; // EOF?
	printf("Sample rate: %dHz\n", sampleRate);

	nbRead=fread(&byteRate, sizeof(int), 1, fp);
	if (nbRead < 1) return 0; // EOF?

	nbRead=fread(&blockAlign, sizeof(short int), 1, fp);
	if (nbRead < 1) return 0; // EOF?

	nbRead=fread(&bitsPerSample, sizeof(short int), 1, fp);
	if (nbRead < 1) return 0; // EOF?
	
	if (bitsPerSample!=8)
	{
		printf("Only WAV files with 8-bit samples supported.\n");
		return 0;
	}
	
	fseek(fp,0,SEEK_SET);
	
	return 1; // This is a file we can work with.
}

void print_help (char * prn)
{
#ifdef __unix__ 
	#ifdef __APPLE__
		char spn[]="/dev/cu.usbserial-DN05FVT8";
	#else
		char spn[]="/dev/ttyUSB0";
	#endif
#else
	char spn[]="COM3";
#endif
	printf("Usage examples:\n");
	printf("%s -D%s -w somefile.wav (write 'somefile.wav' to flash via %s)\n", prn, spn, spn);
	printf("%s -D%s -w -I somefile.wav (write 'somefile.wav' to flash via %s, do not check for valid WAV)\n", prn, spn,spn);
	printf("%s -D%s -v somefile.wav (compare 'somefile.wav' and flash)\n", prn, spn);
	printf("%s -D%s -Rotherfile.wav (save content of SPI flash to 'otherfile.wav')\n", prn, spn);
	printf("%s -D%s -P (play the content of the flash memory)\n", prn, spn);
	printf("%s -D%s -P0x20000,12540 (play the content of the flash memory starting at address 0x20000 for 12540 bytes)\n", prn, spn);
	printf("%s -Amyindex.asm somefile.wav (generate asm index file 'myindex.asm' for 'somefile.wav')\n", prn);
	printf("%s -Cmyindex.c somefile.wav (generate C index file 'myindex.c' for 'somefile.wav'.)\n", prn);
	printf("%s -Cmyindex.c -S2000 somefile.wav (same as above but check for 2000 silence bytes.  Default is 512.)\n", prn);
	fflush(stdout);
}

static const unsigned short crc16_ccitt_table[256] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52B5U, 0x4294U, 0x72F7U, 0x62D6U,
    0x9339U, 0x8318U, 0xB37BU, 0xA35AU, 0xD3BDU, 0xC39CU, 0xF3FFU, 0xE3DEU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64E6U, 0x74C7U, 0x44A4U, 0x5485U,
    0xA56AU, 0xB54BU, 0x8528U, 0x9509U, 0xE5EEU, 0xF5CFU, 0xC5ACU, 0xD58DU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76D7U, 0x66F6U, 0x5695U, 0x46B4U,
    0xB75BU, 0xA77AU, 0x9719U, 0x8738U, 0xF7DFU, 0xE7FEU, 0xD79DU, 0xC7BCU,
    0x48C4U, 0x58E5U, 0x6886U, 0x78A7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xC9CCU, 0xD9EDU, 0xE98EU, 0xF9AFU, 0x8948U, 0x9969U, 0xA90AU, 0xB92BU,
    0x5AF5U, 0x4AD4U, 0x7AB7U, 0x6A96U, 0x1A71U, 0x0A50U, 0x3A33U, 0x2A12U,
    0xDBFDU, 0xCBDCU, 0xFBBFU, 0xEB9EU, 0x9B79U, 0x8B58U, 0xBB3BU, 0xAB1AU,
    0x6CA6U, 0x7C87U, 0x4CE4U, 0x5CC5U, 0x2C22U, 0x3C03U, 0x0C60U, 0x1C41U,
    0xEDAEU, 0xFD8FU, 0xCDECU, 0xDDCDU, 0xAD2AU, 0xBD0BU, 0x8D68U, 0x9D49U,
    0x7E97U, 0x6EB6U, 0x5ED5U, 0x4EF4U, 0x3E13U, 0x2E32U, 0x1E51U, 0x0E70U,
    0xFF9FU, 0xEFBEU, 0xDFDDU, 0xCFFCU, 0xBF1BU, 0xAF3AU, 0x9F59U, 0x8F78U,
    0x9188U, 0x81A9U, 0xB1CAU, 0xA1EBU, 0xD10CU, 0xC12DU, 0xF14EU, 0xE16FU,
    0x1080U, 0x00A1U, 0x30C2U, 0x20E3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83B9U, 0x9398U, 0xA3FBU, 0xB3DAU, 0xC33DU, 0xD31CU, 0xE37FU, 0xF35EU,
    0x02B1U, 0x1290U, 0x22F3U, 0x32D2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xB5EAU, 0xA5CBU, 0x95A8U, 0x8589U, 0xF56EU, 0xE54FU, 0xD52CU, 0xC50DU,
    0x34E2U, 0x24C3U, 0x14A0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xA7DBU, 0xB7FAU, 0x8799U, 0x97B8U, 0xE75FU, 0xF77EU, 0xC71DU, 0xD73CU,
    0x26D3U, 0x36F2U, 0x0691U, 0x16B0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xD94CU, 0xC96DU, 0xF90EU, 0xE92FU, 0x99C8U, 0x89E9U, 0xB98AU, 0xA9ABU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18C0U, 0x08E1U, 0x3882U, 0x28A3U,
    0xCB7DU, 0xDB5CU, 0xEB3FU, 0xFB1EU, 0x8BF9U, 0x9BD8U, 0xABBBU, 0xBB9AU,
    0x4A75U, 0x5A54U, 0x6A37U, 0x7A16U, 0x0AF1U, 0x1AD0U, 0x2AB3U, 0x3A92U,
    0xFD2EU, 0xED0FU, 0xDD6CU, 0xCD4DU, 0xBDAAU, 0xAD8BU, 0x9DE8U, 0x8DC9U,
    0x7C26U, 0x6C07U, 0x5C64U, 0x4C45U, 0x3CA2U, 0x2C83U, 0x1CE0U, 0x0CC1U,
    0xEF1FU, 0xFF3EU, 0xCF5DU, 0xDF7CU, 0xAF9BU, 0xBFBAU, 0x8FD9U, 0x9FF8U,
    0x6E17U, 0x7E36U, 0x4E55U, 0x5E74U, 0x2E93U, 0x3EB2U, 0x0ED1U, 0x1EF0U
};


/******************************************************************************/
unsigned short crc16_ccitt(
        const unsigned char     block[],
        unsigned int            blockLength,
        unsigned short          crc)
{
    unsigned int i;

    for(i=0U; i<blockLength; i++){
        unsigned short tmp = (crc >> 8) ^ (unsigned short) block[i];
        crc = ((unsigned short)(crc << 8U)) ^ crc16_ccitt_table[tmp];
    }
    return crc;
}

// CRC-CCITT (XModem) Polynomial: x^16 + x^12 + x^5 + 1 (0x1021). Initial crc must be 0x0000.
unsigned int crc16 (unsigned int crc, unsigned char val)
{
	unsigned char i;
	crc = crc ^ ((unsigned int)val << 8);
	for (i=0; i<8; i++)
	{
		crc <<= 1;
		if (crc & 0x10000) crc ^= 0x1021;
	}
	return crc;
}

int main(int argc, char **argv)
{
	int j, n;
	FILE * fin, * fout;
    int filesize, silcnt, silence=512;
    unsigned char * bigbuff=NULL;
    BOOL b_ID=FALSE, b_write=FALSE, b_index_asm=FALSE, b_index_c=FALSE, b_read=FALSE, b_verify=FALSE, b_play=FALSE, b_test=FALSE, b_check=TRUE;
    int play_start, play_length;
    unsigned int crc;
	
    printf("SPI flash memory programmer using serial protocol. (C) Jesus Calvino-Fraga (2012-2021)\n");
    fflush(stdout);
	
    for(j=1; j<argc; j++)
    {
    	     if(EQ("-?", argv[j])) { print_help(argv[0]); exit(0); }
    	else if(EQ("-W", argv[j])) b_write=TRUE;
    	else if(EQ("-V", argv[j])) b_verify=TRUE;
    	else if(EQ("-T", argv[j])) b_test=TRUE;
    	else if(EQ("-I", argv[j])) b_check=FALSE;
    	else if(EQ("-M", argv[j])) b_ID=TRUE;
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='W'))
    	{
    		b_write=TRUE;
    		strcpy(InName, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='V'))
    	{
    		b_verify=TRUE;
    		strcpy(InName, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='S'))
    	{
    		silence=atoi(&argv[j][2]);
    		if(silence<20) silence=20;
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='A'))
    	{
    		b_index_asm=TRUE;
    		strcpy(OutNameAsm, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='D'))
    	{
    		strcpy(SerialPort, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='C'))
    	{
    		b_index_c=TRUE;
    		strcpy(OutNameC, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='R'))
    	{
    		b_read=TRUE;
    		strcpy(OutNameRead, &argv[j][2]);
    	}
    	else if((argv[j][0]=='-') && (toupper(argv[j][1])=='P'))
    	{
    		b_play=TRUE;
    		play_start=0;
    		play_length=0;
    		sscanf(&argv[j][2], "%i,%i", &play_start, &play_length);
    		if (play_start<0) play_start=0;
    		if (play_length<0) play_length=0;
    	}
    	else strcpy(InName, argv[j]);
	}
	
	if(argc<2)
	{
		print_help(argv[0]);
		exit(0);
	}
	
	if(b_ID) // Display the three identification bytes for the memory and get out. 
	{
	    if(OpenSerialPort(SerialPort, 115200, NOPARITY, 8, ONESTOPBIT)!=0)
	    {
	        printf("ERROR: Could not open serial port '%s'.\n", SerialPort);
	        fflush(stdout);
	        exit(3);
	    }
		Identify();
		CloseSerialPort();
		return 0;
	}
	   	
	if(b_write || b_index_asm || b_index_c || b_verify || b_test)
	{
	    if(strlen(InName)==0)
	    {
	    	print_help(argv[0]);
	    	return 1;
	    }
	       
		fin = fopen(InName, "rb");
		
		if (fin == NULL)
		{
			printf("Invalid filename '%s'.\n", InName);
			exit(1);
		}
		
		if(b_check)
		{	
			if(Check_Wav(fin)==0)
			{
				printf("The format of '%s' is not supported.\n", InName);
				fclose(fin);
				exit(1);
			}
		}
			
		filesize=fsize(fin);
			
		bigbuff = (unsigned char*) malloc(filesize);
		if(bigbuff==NULL)
		{
			printf("Memory allocation for %d bytes failed.\n", filesize);
			exit(2);
		}
			
		n=fread(bigbuff, sizeof(unsigned char), filesize, fin);
			
			
		if(n!=filesize)
		{
			printf("Error loading file.\n");
			exit(2);
		}
		fclose(fin);
			
		printf("Loaded %d bytes from file '%s'.\n", n, InName); fflush(stdout);
	}
	
	if(b_write || b_read || b_verify || b_play || b_test)
	{	  
	    if(OpenSerialPort(SerialPort, 115200, NOPARITY, 8, ONESTOPBIT)!=0)
	    {
	        printf("ERROR: Could not open serial port '%s'.\n", SerialPort);
	        fflush(stdout);
	        exit(3);
	    }
	}
	
	if(b_write==TRUE)
	{
		if (m_memsize==0) Identify();
		
		if(filesize>m_memsize)
		{
	        printf("The SPI flash memory capacity of %d bytes is insufficient for file '%s' which has a size of %d bytes\n",
	                m_memsize, InName, filesize);
			free(bigbuff);
			CloseSerialPort();
	        exit(3);
		}
		
	    Flash(bigbuff, filesize);
	}
	
	if(b_read==TRUE)
	{
	    int address, j;
	    unsigned char received[0x200];
	    
		START;
		
		printf("Creating output file '%s'\n", OutNameRead); fflush(stdout);
		fout = fopen(OutNameRead, "wb");

		if (fout == NULL)
		{
			printf("Couldn't create file '%s'.\n", OutNameRead);
			free(bigbuff);
			exit(1);
		}
		
	    Identify();
  
		printf("Reading\n"); fflush(stdout);
		for(address=0, j=0; address<m_memsize; address+=256)
		{
			Read_Flash(address, received, 256);
			fwrite(received, sizeof(unsigned char), 256, fout);
			printf(".");
			if (++j==64)
			{
				j=0;
				printf("\n");
			}
			fflush(stdout);
		}
		fclose(fout);
		
		printf(" done\n");
	    printf("\nActions completed in ");
	    STOP;
		PRINTTIME;
		printf("\n"); fflush(stdout);
	}
	
	if(b_verify==TRUE) // Faster verify method that uses a calculated crc
	{
		int flash_crc, calculated_crc, i, temp, n;
		
		n=filesize;
		
		START;
		printf("Verifying SPI flash content"); fflush(stdout);

		//for(i=0, calculated_crc=0; i<n; i++)
		//{
		//	calculated_crc=crc16(calculated_crc, bigbuff[i]);
		//}

		calculated_crc=crc16_ccitt(bigbuff, n, 0);
        		
		calculated_crc&=0xffff;
		flash_crc=get_crc16(n)&0xffff;
		
		if( flash_crc == calculated_crc )
		{
			printf("The CRC of the file and the SPI flash memory is the same: 0x%04x.\n", flash_crc);
		}
		else
		{
			printf("ERROR.  CRC mismatch. SPI flash CRC=0x%04x, File CRC=0x%04x.\n", flash_crc, calculated_crc);
		}
		fflush(stdout);
	    printf("Actions completed in ");
	    STOP;		
		PRINTTIME;
		printf("\n"); fflush(stdout);
	}

	if(b_test==TRUE) // Very slow 'verify' that compares every byte
	{
		int address, j, k;
	    unsigned char received[0x200];
	    BOOL b_error=FALSE;
		
		START;
		
		printf("Comparing flash content to file '%s'... ", InName); fflush(stdout);
		
	    Identify();
  
		for(address=0, j=0; address<filesize; address+=256)
		{
			n=((filesize-address)>256)?256:(filesize-address);
			Read_Flash(address, received, n);
			for(k=0; k<n; k++)
			{
				if(bigbuff[address+k]!=received[k])
				{
					printf("Error at address 0x%06x\n", address+k);
					fflush(stdout);
					address=filesize; // To get out
					b_error=TRUE;
					break;
				}
			}
			printf(".");
			if (++j==64)
			{
				j=0;
				printf("\n");
			}
			fflush(stdout);
		}
		
		printf(b_error?"\nDone\n":"\nMemory and file match.\n");
	    printf("\nActions completed in ");
	    STOP;
		PRINTTIME;
		printf("\n"); fflush(stdout);
	}
	
	if(b_play==TRUE)
	{
		if (m_memsize==0) Identify();
	    fflush(stdout);
	    
	    if(play_length<=0) play_length=m_memsize-play_start;
	    if(play_length>m_memsize) play_length=m_memsize;
	    printf("Playing content of flash from 0x%06x to 0x%06x (%d bytes). \n", play_start, play_start+play_length, play_length);
	    play_stored(play_start, play_length);
	}

	if(b_index_c==TRUE)
	{
		int address;
		printf("Creating 'c' index file '%s'... ", OutNameC); fflush(stdout);
		fout = fopen(OutNameC, "w");

		if (fout == NULL)
		{
			printf("Couldn't create file '%s'.\n", OutNameC);
			free(bigbuff);
			exit(1);
		}
	
		fprintf(fout, "// Approximate index of sounds in file '%s'\n", InName);
		fprintf(fout, "code const unsigned long int wav_index[]={\n");
		for (address = 0, j=0 ; address < filesize ; address++)
		{
			// Look for silence
			if ( (bigbuff[address]>ZERO_MIN) && (bigbuff[address]<ZERO_MAX) )
			{
				silcnt++;
			}
			else
			{
				silcnt=0;
			}
			
			if(silcnt==silence)
			{
			    fprintf(fout, "    0x%06x, // %d \n", address-silcnt, j++);
			    silcnt=0;
				// Look for sound
				for (; address < filesize; address++)
				{
					if ( (bigbuff[address]<=ZERO_MIN) || (bigbuff[address]>=ZERO_MAX) ) break;
				}
			}
		}
		fprintf(fout, "    0x%06x\n};\n", filesize);
		fclose(fout);
		printf("Done.\n"); fflush(stdout);
	}

	if(b_index_asm==TRUE)
	{
		int index;
		int address, previous, sound_size;
		
		printf("Creating 'asm' index file '%s'... ", OutNameAsm); fflush(stdout);
		fout = fopen(OutNameAsm, "w");

		if (fout == NULL)
		{
			printf("Couldn't create file '%s'.\n", OutNameAsm);
			free(bigbuff);
			exit(1);
		}
	
		fprintf(fout, "; Approximate index of sounds in file '%s'\n", InName);
		fprintf(fout, "sound_index:\n");
		for (address = 0, j=0 ; address < filesize ; address++)
		{
			// Look for silence
			if ( (bigbuff[address]>ZERO_MIN) && (bigbuff[address]<ZERO_MAX) )
			{
				silcnt++;
			}
			else
			{
				silcnt=0;
			}
			
			if(silcnt==silence)
			{
				index=address-silcnt;
			    fprintf(fout, "    db 0x%02x, 0x%02x, 0x%02x ; %d \n", (index>>16)&0xff, (index>>8)&0xff, (index>>0)&0xff, j++);
			    silcnt=0;
				// Look for sound
				for (; address < filesize; address++)
				{
					if ( (bigbuff[address]<=ZERO_MIN) || (bigbuff[address]>=ZERO_MAX) ) break;
				}
			}
		}
		fprintf(fout, "    db 0x%02x, 0x%02x, 0x%02x \n", (filesize>>16)&0xff, (filesize>>8)&0xff, (filesize>>0)&0xff);

		fprintf(fout, "\n; Size of each sound in 'sound_index'\n");
		fprintf(fout, "Size_sound:\n");
		for (address = 0, j=0, previous=0; address < filesize ; address++)
		{
			// Look for silence
			if ( (bigbuff[address]>ZERO_MIN) && (bigbuff[address]<ZERO_MAX) )
			{
				silcnt++;
			}
			else
			{
				silcnt=0;
			}
			
			if(silcnt==silence)
			{
				index=address-silcnt;
				if(previous==0)
				{
					previous=index;
				}
				else
				{
					sound_size=index-previous;
					previous=index;
			    	fprintf(fout, "    db 0x%02x, 0x%02x, 0x%02x ; %d \n", (sound_size>>16)&0xff, (sound_size>>8)&0xff, (sound_size>>0)&0xff, j++);
			    }
			    silcnt=0;
				// Look for sound
				for (; address < filesize; address++)
				{
					if ( (bigbuff[address]<=ZERO_MIN) || (bigbuff[address]>=ZERO_MAX) ) break;
				}
			}
		}
		sound_size=filesize-previous;
		previous=filesize;
		fprintf(fout, "    db 0x%02x, 0x%02x, 0x%02x ; %d \n", (sound_size>>16)&0xff, (sound_size>>8)&0xff, (sound_size>>0)&0xff, j);
		fclose(fout);
		printf("Done.\n"); fflush(stdout);
	}
    
    if(bigbuff!=NULL) free(bigbuff);

	if(b_write || b_read || b_verify || b_play || b_test)
	{
		CloseSerialPort();
    }

    return 0;
}
