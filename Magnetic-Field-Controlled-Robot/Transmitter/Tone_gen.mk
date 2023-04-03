SHELL=cmd
CC=c51
COMPORT = $(shell type COMPORT.inc)
OBJS=Tone_gen.obj Startup.obj lcd.obj tetris.obj

tetris.hex: $(OBJS) 
	$(CC) $(OBJS)
	@echo Done!
	
tetris.obj: Tone_gen.c globals.h
	$(CC) -c Tonge_gen.c

Tone_gen.hex: $(OBJS)
	$(CC) $(OBJS)
	@echo Done!
	
Tone_gen.obj: Tone_gen.c globals.h lcd.h
	$(CC) -c Tone_gen.c

Startup.obj: Startup.c globals.h
	$(CC) -c Startup.c

lcd.obj: lcd.c lcd.h globals.h
	$(CC) -c lcd.c

EFM8_I2C_Nunchuck.hex: $(OBJS)
	$(CC) $(OBJS)
	@echo Done!
	
EFM8_I2C_Nunchuck.obj: Tone_gen.c
	$(CC) -c Tone_gen.c

clean:
	@del $(OBJS) *.asm *.lkr *.lst *.map *.hex *.map 2> nul

LoadFlash:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	EFM8_prog.exe -ft230 -r Tone_gen.hex
	cmd /c start putty -serial $(COMPORT) -sercfg 115200,8,n,1,N

putty:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	cmd /c start putty -serial $(COMPORT) -sercfg 115200,8,n,1,N

Dummy: Tone_gen.hex Tone_gen.Map
	
explorer:
	explorer .
		