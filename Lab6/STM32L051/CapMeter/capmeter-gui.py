import serial
import tkinter as tk
import json

# Set up the serial connection
ser = serial.Serial('COM3', 115200)  # Replace 'COM3' with the correct port for your device
ser.flushInput()

# Create the GUI window
root = tk.Tk()
root.title("Serial Data")

capacitance_heading = tk.Label(root, text="Capacitance")
capacitance_heading.grid(row=0,column=0)

frequency_heading = tk.Label(root, text="Frequency")
frequency_heading.grid(row=0,column=1)

capmem_


# Create a label to display the received data
capacitance_label = tk.Label(root, text="No capacitance measured yet.")
capacitance_label.grid(row=1,column=0)

frequency_label = tk.Label(root, text="No frequency detected.")
frequency_label.grid(row=1,column=1)

def read_serial():
    # Read a line of data from the serial port
    data = ser.readline().decode('utf-8').rstrip()

    if data:
        # If data was received, decode it as JSON and update the labels with the received data
        json_data = json.loads(data)
        cap = json_data['cap']
        freq = json_data['freq']
        capacitance_label.config(text=cap)
        frequency_label.config(text=freq)

    # Schedule the function to run again after 100ms
    root.after(100, read_serial)

# Start the GUI loop
root.mainloop()

# Start the function to read from the serial port
read_serial()

# Close the serial connection when the program exits
ser.close()