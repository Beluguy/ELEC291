SHELL=cmd
CC=arm-none-eabi-gcc
AS=arm-none-eabi-as
LD=arm-none-eabi-ld

CCFLAGS=-mcpu=cortex-m0 -mthumb -g
ASFLAGS=-mcpu=cortex-m0 -mthumb -g

OBJS= startup.o main.o 

main.elf: $(OBJS)
	$(LD) $(OBJS) -T ../Common/LDscripts/stm32l051xx_simple.ld --cref -Map main.map -o main.elf
	arm-none-eabi-objcopy -O ihex main.elf main.hex
	@echo Success!

main.o: main.c
	$(CC) -c $(CCFLAGS) main.c -o main.o

startup.o: ../Common/Source/startup.c
	$(CC) -c $(CCFLAGS) ../Common/Source/startup.c -o startup.o

clean: 
	@del $(OBJS) 2>NUL
	@del main.elf main.hex main.map 2>NUL
	@del *.lst 2>NUL
	
Flash_Load:
	@taskkill /f /im putty.exe /t /fi "status eq running" > NUL
	@echo ..\stm32flash\stm32flash -w main.hex -v -g 0x0 ^^>loadf.bat
	@..\stm32flash\BO230\BO230 -b >>loadf.bat
	@loadf
 
