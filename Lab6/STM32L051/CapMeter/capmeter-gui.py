import serial
import tkinter as tk
import json

# Set up the serial connection
ser = serial.Serial('COM3',
                    115200,
                    timeout=1,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_TWO,
                    bytesize=serial.EIGHTBITS)  # Replace 'COM3' with the correct port for your device
ser.flushInput()

# Create the GUI window
root = tk.Tk()
root.title("Serial Data")

capacitance_heading = tk.Label(root, text="Capacitance [??]")
capacitance_heading.grid(row=0,column=0)

frequency_heading = tk.Label(root, text="Frequency")
frequency_heading.grid(row=0,column=1)

# Create a label to display the received data
capacitance_label = tk.Label(root, text="No capacitance measured yet.")
capacitance_label.grid(row=1,column=0)

frequency_label = tk.Label(root, text="No frequency detected.")
frequency_label.grid(row=1,column=1)

capmem_heading = tk.Label(root, text="Mem1 Capacitance")
capmem_heading.grid(row=2,column=0)

freqmem_heading = tk.Label(root, text="Mem1 Frequency")
freqmem_heading.grid(row=2,column=1)

capmem_label = tk.Label(root, text="No capacitance measured yet.")
capmem_label.grid(row=3,column=0)

freqmem_label = tk.Label(root, text="No frequency detected.")
freqmem_label.grid(row=3,column=1)

capmem1_heading = tk.Label(root, text="Mem2 Capacitance")
capmem1_heading.grid(row=4,column=0)

freqmem1_heading = tk.Label(root, text="Mem3 Frequency")
freqmem1_heading.grid(row=4,column=1)

capmem1_label = tk.Label(root, text="No capacitance measured yet.")
capmem1_label.grid(row=5,column=0)

freqmem1_label = tk.Label(root, text="No frequency detected.")
freqmem1_label.grid(row=5,column=1)

prevunit = "uF"

def read_serial():
    global prevunit
    # Read a line of data from the serial port
    data = ser.readline().decode('utf-8').rstrip()

    if data:
        # If data was received, decode it as JSON and update the labels with the received data
        capmem1_label.config(text=capmem_label.cget("text"))
        freqmem1_label.config(text=freqmem_label.cget("text"))

        capmem_label.config(text=capacitance_label.cget("text"))
        freqmem_label.config(text=frequency_label.cget("text"))
        
        json_data = json.loads(data)
        cap = json_data['cap']
        freq = json_data['freq']
        unit = json_data['unit']
        capacitance_label.config(text=cap)
        frequency_label.config(text=freq)
        capacitance_heading.config(text="Capacitance [" + unit + "]")
        if (unit != prevunit):
            prevunit = unit
            if (unit == "uF"):
                try:
                    capmem1_label.config(text=str(float(capmem1_label.cget("text")) / 1000.0))
                    capmem_label.config(text=str(float(capmem_label.cget("text")) / 1000.0))
                except:
                    print("lol")
            else:
                try:
                    capmem1_label.config(text=str(float(capmem1_label.cget("text")) * 1000.0))
                    capmem_label.config(text=str(float(capmem_label.cget("text")) * 1000.0))
                except:
                    print("lol")

    # Schedule the function to run again after 100ms
    root.after(100, read_serial)

# Start the function to read from the serial port
read_serial()

# Start the GUI loop
root.mainloop()

# Close the serial connection when the program exits
ser.close()