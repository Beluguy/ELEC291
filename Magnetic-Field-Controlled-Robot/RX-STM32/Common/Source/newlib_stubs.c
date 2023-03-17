#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "../Include/serial.h"

// #define DEBUG_STUBS

#undef errno
extern int errno;

char *__env[1] = { 0 };
char **environ = __env;

void _exit(int status) { while (1); }
int _close(int file) { return -1; }
int _execve(char *name, char **argv, char **env) { errno = ENOMEM; return -1; }
int _fork() { errno = EAGAIN; return -1; }
int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _getpid() { return 1; }
int _kill(int pid, int sig) { errno = EINVAL; return (-1); }
int _link(char *old, char *new) { errno = EMLINK; return -1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _stat(const char *filepath, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
clock_t _times(struct tms *buf) { return -1; }
int _unlink(char *name) { errno = ENOENT; return -1; }
int _wait(int *status) { errno = ECHILD; return -1; }

int _isatty(int file)
{
    switch (file)
    {
	    case STDOUT_FILENO:
	    case STDERR_FILENO:
	    case STDIN_FILENO:
	        return 1;
	    default:
	        errno = EBADF;
	        return 0;
    }
}

// These two "spaces" are defined at the bottom of the linker script lpc824.ld
extern unsigned int _StackTop;   // Top of stack memory.
extern unsigned int _HeapStart; // Bottom of heap, for malloc and such.
#define STACKSIZE 0x1000

caddr_t _sbrk(int incr)
{
    char * _heap_start=(char *) &_HeapStart; // Defined by the linker via linker script
	char * _stack_end=(char *)((char *) &_StackTop - (char *) (STACKSIZE));
    static char * heap_end=0;
    char *prev_heap_end;

	#ifdef DEBUG_STUBS
	printString("Inside _sbrk()");
	#endif
	
    if (heap_end == 0)
    {
		#ifdef DEBUG_STUBS
    	printString("Initializing heap");
    	PRINTVAR(_heap_start);
    	PRINTVAR(_stack_end);
    	PRINTVAR(&_vStackTop);
    	PRINTVAR(&_pvHeapStart);
		#endif
        heap_end = _heap_start;
    }
    prev_heap_end = heap_end;

	if (heap_end + incr >  _stack_end)
	{
		#ifdef DEBUG_STUBS
		printString("Heap and stack collision\n");
		#endif
		errno = ENOMEM;
		return  (caddr_t) -1;
	}

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

int _read(int file, char *ptr, int len)
{
    int n;
    int num = 0;
    volatile char c;

	#ifdef DEBUG_STUBS
	printString("Inside _read()");
	#endif

    switch (file)
    {
	    case STDIN_FILENO:
	        for (n = 0; n < len; n++)
	        {
	            c = egetc(); // Get char from uart.  This function is in serial.c
	            //if(c==0x0d) c=0x0a;
	            *ptr++ = c;
	            num++;
	            if (c==0x0a) return num; // Otherwise gets(), fgets(), scanf(), won't work
	        }
	        break;
	    default:
	        errno = EBADF;
	        return -1;
    }
    return num;
}

int _write(int file, char *ptr, int len)
{
    int n;

	#ifdef DEBUG_STUBS
	printString("Inside _write()");
	#endif

    switch (file)
    {
	    case STDOUT_FILENO:
	    case STDERR_FILENO:
	        for (n = 0; n < len; n++) eputc(*ptr++); // Put char in uart.  This function is in serial.c
	        break;
	    default:
	        errno = EBADF;
	        return -1;
    }

    return len;
}
