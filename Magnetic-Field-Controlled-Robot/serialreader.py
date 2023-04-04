import serial
import subprocess

# Set up the serial connection
ser = serial.Serial('COM3',
                    115200,
                    timeout=1,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_TWO,
                    bytesize=serial.EIGHTBITS)  # Replace 'COM3' with the correct port for your device

while True:
    # Read one line from the serial port
    line = ser.readline().decode('utf-8').rstrip()
    
    if line == 'tetris':
        # Launch the Tetris game script using subprocess
        subprocess.Popen(['python', 'tetrisgame.py'])