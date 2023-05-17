### Hello there, 
Here you can find all the parts that we used for this project. This list should have sufficient detail that someone skilled in the art could reproduce our project.

---

###	Hardware Components:
#### Main chipsets: 
- STMicroelectronics STM32L051
- EFM8LB1 microcontroller board by Jesus Calvino-Fraga

#### Accompanying hardware for robot: 
-	2x0.1uF 50V Capacitors
-	2x1uF 50V Capacitors 
-	2x270 Ohm Resistor 
-	330 Ohm Resistor
-	2xRed LED – Used for Additional Feature
-	Green LED
-	Microchip MCP17003320E 3.3V Voltage regulator 
-	BO230XS serial USB Adaptor
-	QFP32 to DIP32 adapter 
-	2x16-pin header connectors 
-	2xPush Button Switches
-	220uF 50V Capacitor 
-	2x1K Ohm resistor
-	Full size Solderless Breadboard
#### Receiver:
-	4xMBR150 Axial Lead Rectifiers
-	4x0.1uF 50V Capacitors
-	2x470K Ohm Resistors 
-	2x1mH Wirewound Axial Inductor 
-	Fairchild Semiconductor LM358 Dual Operational Amplifier
-	4x 1K Ohm Resistors 
-	2x47K Ohm Resistors 
#### H-bridges: 
-	4x1K Ohm Resistors
-	4x10K Ohm Resistors 
-	LiteON Incorporated LTV847 Optocoupler 
-	4xFairchild Semiconductor FQP8P010 P-FET
-	4xFairchild Semiconductor FQP13N06L N-FET
-	2x0.1uF 50V Capacitors 
#### Robot Chassis Assembly:
-	Ball caster kit
-	2xSolarbotics GM4 Clear servo motor
-	2x3D printed Servo wheels
-	Custom waterjet cut aluminum chassis 
-	4-40 screw/nut kit
-	4xAA batter holder 
-	Lithium-ion battery charger - Used for Additional Feature
-	5V buck converter board - Used for Additional Feature
-	DPDT Switch
#### Accompanying hardware for remote control: 
-	Wii Nunchuk with Adapter - Used for Additional Feature
-	220uF 63V Capacitor
-	220uF 50V Capacitor 
-	2x0.1uF 50V Capacitor 
-	Generic Arduino IR Obstacle Avoidance Sensor Module - Used for Additional Feature
-	MC7800 5V Voltage Regulator 
-	Microchip MCP17003320E 3.3V Voltage regulator 
-	CTS Electrocomponents 206-4 through hole dip switch
-	STMicroelectronics LM335 Temperature Sensor - Used for Additional Feature
#### Battery Capacity Sensor: - Additional Feature
-	330K Ohm Resistor
-	270 Ohm Resistor
-	22K Ohm Resistor
-	15K Ohm Resistor
#### Transmitter:
-	0.1uF 300V Capacitor
-	1mH Wirewound Axial Inductor 
#### H-bridge: 
-	4x1K Ohm Resistors
-	2x10K Ohm Resistors 
-	2x2N3903 NPN BJT transistors
-	2xFairchild Semiconductor FQP8P010 P-FET
-	2xFairchild Semiconductor FQP13N06L N-FET

### Software Features
Base Functionality: 
	Robot:
-	Constantly reads commands wireless using a pair of inductors via magnetic field from the remote control 
-	The command encoding system is one-hot to minimize the delay between movement and trigger on the remote by prioritizing the most used commands to be the shortest  
-	The one-hot encoding system supports the following seven commands:
  -	Turn left
  -	Turn right
  -	Go forward
  -	Backward
  -	Switch mode 
-	Once in automatic/tracking mode, the robot is able to adjust its relative position and angle with respect to the controller 
-	The electromagnetic signal is operated at a constant frequency of 16.275kHz
-	Both the robot and the remote control are coded using the C programming language 
Remote Controller:
- The controller transmits one-hot commands via magnetic field mentioned above
- Generates a sinusoidal wave at a frequency of 16.275kHz
-	Sends the following one-hot command when triggered:
  -	Turn left
  -	Turn right
  -	Go forward
  -	Backward
  -	Switch mode 
  
#### Additional Features:
This sectioned was removed to encourge creativity of student when doing their version of the reflow oven controller.
