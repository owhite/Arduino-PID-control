# Arduino-PID-control

This is a project to utilize the arduino PID library to run a dc motor, and then graph the results of the motor response using python's pyplot.

The [arduino playground](http://playground.arduino.cc/Code/PIDLibrary) page for the PID library.

Installation and usage of python as well as pyplot is left up to you.

Imagine you wanted to move a use a DC motor and get that thing to rotate to a specific position.

You hook up the motor to your to arduino using an L298 motor driver ([example](http://www.instructables.com/id/Control-DC-and-stepper-motors-with-L298N-Dual-Moto/)). Speed of the motor is controlled by the stupidly easy [PWM methods](https://www.arduino.cc/en/Tutorial/PWM). The PWM settings is guided manually to move using a potentiometer.

This graph is an example of what might happen:

![Movement attempt](http://i.imgur.com/lCbXkjo.jpg) 

The x-axis is time, the y-axis is motor position. The red line shows the position you want to move the motor. But it's just you and your fat fingers trying to move that motor. What might happen is something like the blue line. You request to go to a certain position and in the case of the blue line, the motor wildly over shoots that position. You dont know how to slow down at the right time, maybe if you were manually controlling it it would go "whoaaaaa" and try to swing back to that position, over shooting, and eventually you'd bounce back and forth to get close to the desired position.

Suppose then, you're like me and you [read up on dc motor control](https://www.youmagine.com/designs/dc-motor-closed-loop-control-software) using an arduino and you discover something called PID. In this case you send serial commands to the motor, no more potentiometers. And also let's go with a [dc motor](http://i.imgur.com/485PtIJ.jpg) equipped with an encoder. "Hey hey, great resolution" you think. Here, you use the encoder to track the position based on the number of encoder ticks and you hope the motor will arrive at that position. 

The original [Grett Beauregard blog post](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/) on PID for the arduino.

So I have to admit I never really understood this formula:  

![PID formula](http://i.imgur.com/VzkznFA.png)

still dont, quite frankly. But once you get PID working in your arduino code, and try to move a motor - this is an example of what might happen:  

![Wikipedia PID graph](https://upload.wikimedia.org/wikipedia/commons/a/a3/PID_varyingP.jpg)

Actually this is what will never happen without PID. Again, the x-axis is time, the y-axis is motor position. Using a serial command you issue a command to go to a certain position. The blue shows the ideal situation: at a certain time point you want the dumb thing to go to a specific point and just stay there. What might happen is something like the purple line. You request to go to a certain position and in the case of the purple line, the motor wildly over shoots that position. It also doesnt know how to slow down at the right time, it also goes "whoaaaaa" and tries to swing back to the target position, over shooting, and eventually bounces back and forth to get close to the target position.


