#define SYSCLK    72000000L // SYSCLK frequency in Hz 32000000
#define BAUDRATE  115200L   // Baud rate of UART in bps
#define DEFAULT_F 15500L
#define SARCLK 18000000L
#define SMB_FREQUENCY  100000L   // I2C SCL clock rate (10kHz to 100kHz)

#define OUT0 P2_0
#define OUT1 P2_1
#define DISPLAY P1_7
#define FORWARD P1_4
#define LEFT P1_2
#define RIGHT P1_0
#define BACKWARD P0_5
#define SWITCHER P0_3
#define VDD 3.3035 // The measured value of VDD in volts