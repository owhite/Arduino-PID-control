# Arduino-PID-control

This is a project to utilize the arduino PID library to run a dc motor, and then graph the results of the motor response using python's pyplot.

The [arduino playground](http://playground.arduino.cc/Code/PIDLibrary) page for the PID library.

The original [Grett Beauregard blog post](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/) on PID for the arduino.

Installation and usage of python as well as pyplot is left up to you.

So I have to admit I never really understood this formula:  

![PID formula](http://i.imgur.com/VzkznFA.png)

still dont, quite frankly. But these plots are making a lot more sense.

![Wikipedia PID graph](https://upload.wikimedia.org/wikipedia/commons/a/a3/PID_varyingP.jpg)

Okay, so suppose you wanted to move a use a DC motor and get that thing to rotate to a specific position. 

