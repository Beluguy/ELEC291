import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math
import time 
import serial 
import pyttsx3

# Initialize the pyttsx3 engine
engine = pyttsx3.init()

# Set the voice property of the engine
voices = engine.getProperty('voices')
engine.setProperty('voice', voices[0].id) # 0 represents the index of the voice in the list of available voices

colour="blue"

xsize=360
sectimer = 0

# configure the serial port 
ser = serial.Serial( 
 port='COM7', 
 baudrate=115200, 
 parity=serial.PARITY_NONE, 
 stopbits=serial.STOPBITS_TWO, 
 bytesize=serial.EIGHTBITS 
) 
   
def data_gen():
    global sectimer, colour
    t = data_gen.t
    while True:
        t+=1
        val=int(ser.readline())
        state=int(ser.readline())
        yield t, val
        
        if val == -1:
            engine.say("oven is now cool, pcb may still be hot")
            engine.runAndWait()
            quit()
        if state == -1:
            engine.say("oven is now cool, pcb may still be hot")
            engine.runAndWait()
            quit()
        
        sectimer=sectimer+1
        if sectimer >= 10:
            sectimer = 0
            # Convert text to speech and play it
            engine.say(str(val) + " degrees celsius")
            engine.runAndWait()
            engine.say("state: ")
            engine.runAndWait()
            if state == 1:
                engine.say("ramp to soak")
                engine.runAndWait()
                line.set_color("blue")
            elif state == 2:
                engine.say("soak")
                engine.runAndWait()
                line.set_color("green")
            elif state == 3:
                engine.say("ramp to peak")
                engine.runAndWait()
                line.set_color("red")
            elif state == 4:
                engine.say("reflow")
                engine.runAndWait()
                line.set_color("yellow")
            elif state == 5:
                engine.say("cooling")
                engine.runAndWait()
                line.set_color("orange")
            else:
                print(state)

def run(data):
    # update the data
    t,y = data
    if t>-1:
        xdata.append(t)
        ydata.append(y)
        if t>xsize: # Scroll to the left.
            ax.set_xlim(t-xsize, t)
        line.set_data(xdata, ydata)

    return line,

def on_close_figure(event):
    sys.exit(0)

ser.isOpen()
data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw=2, color=colour)
ax.set_ylim(0, 300)
ax.set_xlim(0, xsize)
ax.grid()
xdata, ydata = [], []

# Important: Although blit=True makes graphing faster, we need blit=False to prevent
# spurious lines to appear when resizing the stripchart.
ani = animation.FuncAnimation(fig, run, data_gen, blit=False, interval=100, repeat=False)
plt.show()