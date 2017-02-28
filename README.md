# Arduino-PID-control

## INTRODUCTION

Imagine you wanted to move a use a DC motor and get that thing to rotate to a specific position.

You hook up the motor to your to arduino using an L298 motor driver ([example](http://www.instructables.com/id/Control-DC-and-stepper-motors-with-L298N-Dual-Moto/)). Speed of the motor is controlled by the stupidly easy [PWM methods](https://www.arduino.cc/en/Tutorial/PWM). The PWM settings for example could be guided manually to move using a potentiometer - [example](http://www.instructables.com/id/Controlling-Motors-with-Arduino/).

This graph is an example of what might happen:

![Movement attempt](http://i.imgur.com/lCbXkjo.jpg) 

The x-axis is time, the y-axis is motor position. The red line shows the position you want to move the motor. But it's just you and your fat fingers trying to move that motor. What might happen is something like the blue line. You request to go to a certain position and in the case of the blue line, the motor wildly over shoots that position. You dont know how to slow down at the right time, maybe if you were manually controlling it it would go "whoaaaaa" and try to swing back to that position, over shooting, and eventually you'd bounce back and forth to get close to the desired position.

Suppose then, you're like me and you [read up on dc motor control](https://www.youmagine.com/designs/dc-motor-closed-loop-control-software) using an arduino and you discover something called PID. In this case you send serial commands to the motor, no more potentiometers. And also let's go with a [dc motor](http://i.imgur.com/485PtIJ.jpg) equipped with an encoder. "Hey hey, great resolution" you think. Here, you use the encoder to track the position based on the number of encoder ticks and you hope the motor will arrive at that position. 

The original [Grett Beauregard blog post](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/) on PID for the arduino.

This is the [arduino playground](http://playground.arduino.cc/Code/PIDLibrary) page for the PID library.

So I have to admit I never really understood this formula:  

![PID formula](http://i.imgur.com/VzkznFA.png)

still dont, quite frankly. But once you get PID working in your arduino code, and try to move a motor - this is an example of what might happen:  

![Wikipedia PID graph](https://upload.wikimedia.org/wikipedia/commons/a/a3/PID_varyingP.jpg)

Again, the x-axis is time, the y-axis is motor position. Using a serial you issue a command to go to a certain position. The blue shows the ideal situation: at a certain time point you want the dumb thing to go to a specific point and just stay there. What might happen is something like the purple line. You request to go to a certain position and in the case of the purple line, the motor wildly over shoots that position. It also doesnt know how to slow down at the right time, it also goes "whoaaaaa" and tries to swing back to the target position, over shooting, and eventually bounces back and forth to get close to the target position.

You try a bunch of variables for kP, kI, and kD and the motor is behaving erratically. The other lines on the graph show various behaviors depending on your settings. Okay, so this sucks - because none of the PID components work well unless you are able to really see how the motor is reacting to the PID settings. Maybe you've got enough experience with tuning that you've developed an intuition for how to set parameters to work well, or, maybe you'll just luck out. Not me, not so far. 

## Enter: somewhat more rational PID tunings for an arduino-based dc motor.

If you use python, you can use it to issue commands to an arduino, change PID settings, and then graph the results to view the response of the motor. 

Here's the plan: 
1  Write an IDE that can take commands from python
1  Use the python code on the command line to connect through the serial to your curcuit
1  Have the arduino do a little data recording
1  Watch the motor behavior
1  Issue various commands to control the circuit
..* Change PID settings
..* Store settings in eeprom
..* Dump data to pyplot

Installation and usage of python as well as pyplot is left up to you.

