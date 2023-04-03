SHELL=cmd
CC=c51
COMPORT = $(shell type COMPORT.inc)
OBJS=Tone_gen.obj Startup.obj

Tone_gen.hex: $(OBJS)
	$(CC) $(OBJS)
	@echo Done!
	
Tone_gen.obj: Tone_gen.c globals.h
	$(CC) -c Tone_gen.c

Startup.obj: Startup.c globals.h
	$(CC) -c Startup.c

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
		