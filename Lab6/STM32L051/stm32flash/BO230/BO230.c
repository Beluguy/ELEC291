/////////////////////////////////////////////////////////////////////////////
//
//  BO230.c:  FT230XS Boot activator for STM32l0xx
// 
// Copyright (C) 2012-2022  Jesus Calvino-Fraga, jesusc (at) ece.ubc.ca
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
// Compile using Visual C on Windows:
// cl BO230.c
//
// Make sure ftd2xx.dll, ftd2xx.h, and ftd2xx.lib are present in the same folder.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma comment(lib, "FTD2XX.lib")
#define FTD2XX_STATIC
#include "ftd2xx.h"

int Selected_Device=-1;
FT_HANDLE handle=NULL;

#define EQ(a,b) (stricmp(a,b)==0)

void GetOut (int val)
{
	if(handle!=NULL)
	{
		FT_SetBitMode(handle, 0x00, FT_BITMODE_CBUS_BITBANG); // Set CBUS3 and CBUS0 as input
		FT_SetBitMode(handle, 0x0, FT_BITMODE_RESET); // Back to serial port mode
		FT_Close(handle);
	}
	exit(val);
}

// CBUS3 and CBUs0 are used as RESET and BOOT respectevely, to activate bootload mode.
// They must be configured for GPIO first.
void FTDI_Set_CBUS_Mode (int pinmode)
{
	FT_STATUS status;
	char Manufacturer[64];
	char ManufacturerId[64];
	char Description[64];
	char SerialNumber[64];
	int j;
	
	FT_EEPROM_HEADER ft_eeprom_header;
	FT_EEPROM_X_SERIES ft_eeprom_x_series;
	
	ft_eeprom_header.deviceType = FT_DEVICE_X_SERIES; // FTxxxx device type to be accessed
	ft_eeprom_x_series.common = ft_eeprom_header;
	ft_eeprom_x_series.common.deviceType = FT_DEVICE_X_SERIES;
	
	status = FT_EEPROM_Read(handle, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series),
							Manufacturer, ManufacturerId, Description, SerialNumber);
	// FT_X_SERIES_CBUS_IOMODE configure pin to this mode for bit bang mode
	// FT_X_SERIES_CBUS_SLEEP Factory default setting for CBUS3
	// FT_X_SERIES_CBUS_TXLED Factory default setting for CBUS2
	// FT_X_SERIES_CBUS_RXLED Factory default setting for CBUS1
	// FT_X_SERIES_CBUS_TXDEN Factory default setting for CBUS0
	if (status == FT_OK)
	{
		if(pinmode==1)
		{
			if ((ft_eeprom_x_series.Cbus3!=FT_X_SERIES_CBUS_IOMODE) ||  (ft_eeprom_x_series.Cbus0!=FT_X_SERIES_CBUS_IOMODE))
			{
				ft_eeprom_x_series.Cbus3=FT_X_SERIES_CBUS_IOMODE;
				ft_eeprom_x_series.Cbus0=FT_X_SERIES_CBUS_IOMODE;
				status = FT_EEPROM_Program(handle, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series),
										Manufacturer, ManufacturerId, Description, SerialNumber);
				if (status == FT_OK)
				{
					FT_CyclePort(handle); // It takes about 5 seconds to cycle port
					FT_Close(handle);
					printf("Pins CBUS3 and CBUS0 configured as 'IO Mode for CBUS bit-bang'.\n");					
					fflush(stdout);
					
					for(j=0; j<20; j++)
					{
						if(FT_Open(List_FTDI_Devices(), &handle) != FT_OK)
						{
							Sleep(1000);
							printf(".");
							fflush(stdout);
						}
						else
						{
							printf("\n");
							fflush(stdout);
							break;
						}
					}
					if(j==20)
					{
				        printf("Can not open FTDI adapter.\n");
						fflush(stdout);
						exit(1);
					}    
				}
			}
		}
		else
		{
			if ((ft_eeprom_x_series.Cbus3!=FT_X_SERIES_CBUS_SLEEP) || (ft_eeprom_x_series.Cbus0!=FT_X_SERIES_CBUS_TXDEN))
			{
				ft_eeprom_x_series.Cbus3=FT_X_SERIES_CBUS_SLEEP;
				ft_eeprom_x_series.Cbus0=FT_X_SERIES_CBUS_TXDEN;
				status = FT_EEPROM_Program(handle, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series),
										Manufacturer, ManufacturerId, Description, SerialNumber);
				if (status == FT_OK)
				{
					FT_ResetDevice(handle);
					Sleep(100);
					printf("WARNING: Pins CBUS3 and CBUS0 have been configured as factory default.\n");
					printf("Please unplug/plug the BO230XS board for the changes to take effect\n"
					       "and try again.\n");
					fflush(stdout);
					GetOut(0);
				}
			}
		}
	}
}

void Activate_Bootloader (void)
{
	// Bits 7 down to 4 set CBUSx as either input or ouput.  Bits 3 down to 0 is the actual output.
	// For example 0x90 configures CBUS3 and CBUS0 as outputs and set their values to zero.
	unsigned char Seq[3]={0x91, 0x99, 0x98};
	unsigned char c;

	//printf("Boot\n");
    FT_SetBaudRate(handle, 115200);
	
	FTDI_Set_CBUS_Mode(1); // Make sure both CBUS3 and CBUS0 are in "CBUS IOMODE"
	
	for(c=0; c<3; c++)
	{
		FT_SetBitMode(handle, Seq[c], FT_BITMODE_CBUS_BITBANG); // RESET=0, TEST=0
		Sleep(10); // Sleep mili seconds
	}
	
	FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
	FT_SetBitMode(handle, 0x00, FT_BITMODE_CBUS_BITBANG); // Set CBUS3 and CBUS0 as input
	FT_SetBitMode(handle, 0x0, FT_BITMODE_RESET); // Back to serial port mode

	Sleep(10);
}

void Reset (void)
{
	unsigned char Seq[3]={0x80, 0x88, 0x88};
	unsigned char c;

	//printf("Reset\n");
    FT_SetBaudRate(handle, 115200);
	
	FTDI_Set_CBUS_Mode(1); // Make sure both CBUS3 and CBUS0 are in "CBUS IOMODE"
	
	for(c=0; c<3; c++)
	{
		FT_SetBitMode(handle, Seq[c], FT_BITMODE_CBUS_BITBANG); // RESET=0, TEST=0
		Sleep(10); // Sleep mili seconds
	}
	
	FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
	FT_SetBitMode(handle, 0x00, FT_BITMODE_CBUS_BITBANG); // Set CBUS3 and CBUS0 as input
	FT_SetBitMode(handle, 0x0, FT_BITMODE_RESET); // Back to serial port mode

	Sleep(10);
}

int List_FTDI_Devices (void)
{
	FT_STATUS ftStatus;
	FT_HANDLE ftHandleTemp;
	DWORD numDevs;
	DWORD Flags;
	DWORD ID;
	DWORD Type;
	DWORD LocId;
	char SerialNumber[16];
	char Description[64];
	int j, toreturn=0;
	LONG PortNumber;

	if (Selected_Device>=0) return Selected_Device;
	
	// create the device information list
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus == FT_OK)
	{
		//printf("Number of devices is %d\n",numDevs);
	}
	
	if (numDevs > 1)
	{
		printf("More than one device detected.  Use option -d to select device to use:\n");
		for(j=0; j<numDevs; j++)
		{
			ftStatus = FT_GetDeviceInfoDetail(j, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandleTemp);
			if (ftStatus == FT_OK)
			{
				printf("-d%d: ", j);
				//printf("Flags=0x%x ",Flags);
				//printf("Type=0x%x ",Type);
				printf("ID=0x%x ",ID);
				//printf("LocId=0x%x ",LocId);
				printf("Serial=%s ",SerialNumber);
				printf("Description=%s ",Description);
				//printf(" ftHandle=0x%x",ftHandleTemp);
				FT_Open(j, &handle);
				FT_GetComPortNumber(handle, &PortNumber);				
				FT_Close(handle);
				printf("Port=COM%d\n", PortNumber); fflush(stdout);
			}
		}
		fflush(stdout);
		GetOut(-1);
	}
	
	return toreturn;
}

void print_help (char * prn)
{
	printf("Some examples:\n"
	       "%s -b (activate bootloader using CBUS3 as RESET and CBUS0 as BOOT)\n"
	       "%s -r (hard reset using CBUS as RESET)\n",
	       prn, prn);
}

int main(int argc, char ** argv)
{
    int j, k;
	LONG lComPortNumber;
	BOOL b_Boot=FALSE, b_Reset=FALSE ;
	char buff[0x100];

    for(j=1; j<argc; j++)
    {
    	     if(EQ("-b", argv[j])) b_Boot=TRUE;
    	else if(EQ("-r", argv[j])) b_Reset=TRUE;
    	else if(EQ("-h", argv[j])) {print_help(argv[0]); return 0;}
    }

    if(FT_Open(List_FTDI_Devices(), &handle) != FT_OK)
    {
        puts("Can not open FTDI adapter.\n");
        return 3;
    }
    
    FT_GetComPortNumber(handle, &lComPortNumber);
   
    if(b_Boot==TRUE) Activate_Bootloader();
    if(b_Reset==TRUE) Reset();
	
	FT_SetBitMode(handle, 0x00, FT_BITMODE_CBUS_BITBANG); // Set CBUS3 as input
	FT_SetBitMode(handle, 0x0, FT_BITMODE_RESET); // Back to serial port mode
	FT_Close(handle);

	if (lComPortNumber != -1)
	{
		printf("COM%d\n", lComPortNumber); fflush(stdout);
	}
	
    return 0;
}
