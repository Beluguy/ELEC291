### Hello there, 
Here you can find all the parts that we used for this project. This list should have sufficient detail that someone skilled in the art could reproduce our project.

---

###	Hardware Components:
#### Main chipsets: 
-	Atmel AT89LP52 Microcontroller IC 80C51 MCU 8K FLASH 40-DIP
-	Microchip MCP3008 8-Channel 10-Bit ADC with SPI Interface
#### Accompanying hardware: 
-	22.1184MHz CTS ATS122B-E Quartz Crystal
-	3x0.1uF 50V Capacitors
-	Green LED
-	BO230XS serial USB Adaptor
-	6 Push Button Switches
-	330 Ohm Resistor
-	Handson MB102 Breadboard 3.3V/5V Power Supply
-	2x full size Solderless Breadboard
-	9V battery (optional)
-	9V battery to DC barrel jack adapter (optional)
#### Temperature sensing sub-system:
-	Texas Instruments OP07CP Op Amp chip
-	Texas Instruments LM4040 Voltage Reference 
-	STMicroelectronics LM335 Temperature Sensor
-	Microchip LMC7660 Voltage Converter
-	2x1K Ohm Resistors
-	2x100K Ohm Resistors
-	2x10uF 25V Capacitors
-	2-meters general purpose Thermocouple 
#### Sound output sub-system: 
-	Microchip MCP1700 LDO Voltage Regulator
-	Texas Instruments LM386 Low Voltage Audio Power Amplifier
-	Winbond W25Q80 8-bit Flash Memory
-	Generic 1W Speaker
-	3x1uF 50V Capacitors
-	470uF 25V Capacitor 
-	3x1K Ohm Resistors 
-	10 Ohm Resistor
-	Fairchild FQD13N06L NMOS
-	2x0.1uF 50V Capacitors
-	10K Ohm Potentiometer 
#### PWM output sub-system:
-	Fairchild FQU8P10TU PMOS
-	A pair of cable with banana plug and alligator clip on each end
#### LCD display sub-system:
-	Hitachi HD44780 LCD
-	CTS Electrocomponents 206-4 through hole dip switch
-	2x3K Ohm Resistors
-	2x1K Ohm Resistors
-	3.9K Ohm Resistors

###	Software Components & Features:
#### Base Functionality:
-	Temperature Measurement up to 255°C.
-	Oven temperature regulation via PWM
-	Selectable reflow profile parameters including:
-	Soak temperature
-	Soak time
-	Reflow temperature.
-	Reflow time
-	Display of reflow temperature, time elapsed and current reflow process state on an LCD
-	Audio play back of current oven temperature and state every 5 seconds using Python on a Computer
-	Start/Stop push button to start or stop the reflow process at any time
-	Temperature strip chart plot on a computer using data send from the controller via serial
#### Additional Features:
This sectioned was removed to encourge creativity of student when doing their version of the reflow oven controller. 
