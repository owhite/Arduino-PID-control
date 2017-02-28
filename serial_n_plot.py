#!/usr/bin/env python

import time
import sys
import glob
import serial
import signal
import re
import math
import matplotlib.pyplot as plt

class handle_device:
    def __init__(self, device, baud_rate):
        port = '/dev/tty.usbmodem2335471'
        self.port_found = 0
        ser = serial.Serial(port, baud_rate, timeout=5)
        time.sleep(2)
        ser.flush()
        cmd = 'probe_device\n'
        ser.write(cmd)
        time.sleep(.3) 
        buffer = ''
        while ser.inWaiting() > 0:
            buffer += ser.read(1)
        buffer = buffer.rstrip()
        if (buffer == device):
            self.port_found = 1
            self.serial = ser
            self.port_name = port
        else:
            ser.close()

    def handle_plot(self, text):
        x = []    # each loop number
        y = []    # decay results

        max = 0;
        min = 10000;
        for line in text.split('\n'):
            str = line.split(' ')
            if (str[0] == 'data'):
                t = 0
                for s in str[1:]:
	            x.append(t)
	            y.append(int(s))
                    if (int(s) > max): max = int(s)
                    if (int(s) < min): min = int(s)
	            t += 1
            if (str[0] == 'target'):
                target = float(str[1].rstrip())
            if (str[0] == 'start'):
                start = str[1]
            if (str[0] == 'position'):
                position = float(str[1].rstrip())
            if (str[0] == 'kP'):
                kP = float(str[1].rstrip())
            if (str[0] == 'kI'):
                kI = float(str[1].rstrip())
            if (str[0] == 'kD'):
                kD = float(str[1].rstrip())

        height = max - min
        bump = height * 0.05
        axes = plt.gca()
        axes.set_ylim([min - bump, max + bump])
        PID_str = ("kP = %s kI = %s kD = %s") % (kP, kI, kD)
        # plt.text(20, max * .8, ('position = %s' % position), fontsize=10)
        # plt.text(20, max * .75, ('target = %s' % target), fontsize=10)
        # plt.text(20, max * .70, PID_str, fontsize=10)

        plt.text((t / 2), min + bump, PID_str,
                 horizontalalignment='left',
                 verticalalignment='bottom')
        
        plt.plot(x, y, label=('position = %s' % int(position)))
        plt.plot([0, t], [target, target], label=('target = %s' % int(target)))
        plt.legend(loc=1); 
        plt.show()

        
class Decay:
    def __init__(self):
        # Using radioactive decay for acceleration
        #  where the count of atoms is used as the length of delay between
        #  each loop moving a motor. Decay of the atoms over the loops
        #  speeds up the motor
        self.accelTime = 600    # Number of loops to arrive at velocity
        self.startDelay = 40000	# Number of atoms at t = 0, starting velocity
        self.finalDelay = 100   # Number of atoms to decay to, ending velocity
        self.decayConstant = .01

        # find final decay point using the above parameters
        N = float(self.startDelay)
        L = self.decayConstant
        t = 0
        while t < self.accelTime:
	    N = N - (L * N)
            t += 1
        self.endPoint = N

        # The issue is the decay of our atoms may be much larger or smaller
        #  than what we want. But presumably we love the rate. So
        # Scale our final end point to our desired endpoint
        #  do this by getting ranges
        self.leftSpan = self.startDelay - self.endPoint
        self.rightSpan = self.startDelay - self.finalDelay

    def calcDecay(self, N):
        # this is the loss of atoms
        N = N - (d.decayConstant * N)
        # now scale our current value to our desired endpoint
        s = float(N - self.endPoint) / float(self.leftSpan)
        return (N, self.finalDelay + (s * self.rightSpan))

if __name__ == "__main__":

    l = handle_device('PID_device', 115200)
    if l.port_found:
        port = l.port_name
        print "found: " , port
        ser = l.serial
        plot_toggle = True
        while True:
            buffer = []
            try:
                for message in iter(lambda:raw_input("Enter Message:"),""):
                    if (message == 'quit'):
                        print message
                        ser.close()
                        sys.exit()
                    msg_with_newline = message+"\n"
                    print "sending", msg_with_newline.rstrip()
                    ser.write(msg_with_newline)
                    buffer = ''
                    time.sleep(1)
                    while ser.inWaiting() > 0:
                        buffer += ser.read(1)
                    if (re.match(r'plot', buffer)):
                        l.handle_plot(buffer)                        
                    else:
                        print ("%s %s") % (port, buffer)

            except KeyboardInterrupt:
                print "Bye"
                ser.close()
                sys.exit()

