#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int main (int argc, char ** argv)
{
	if (argc>1) 
	{
		Sleep(atoi(argv[1]));
	}
	else
	{
		Sleep(2000);
	}
	return 0;
}
