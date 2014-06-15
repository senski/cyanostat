Author: Stephanie Conley
Date: June 14, 2014


Description:

Hello! The code in this repository is loaded onto a Arduino Mega.
After this is achieved, the system is pretty much ready to start
processing. If this is moving too fast and you are new to Arduino, then
visit this how-to page:

http://arduino.cc/en/Guide/HomePage

This link will take you to Ardunio's official website and will give
a nice tutorial on how to get started, install software, load code,
etc. 

So, what does the code in this repository capable of?

The code has differ sections and libraries that define certain tasks.
The beginning portion of the code is where the user can make code changes.
This is, perhaps, the only part of the code that the user will ever
need to change. The user only changes numerical values of functions
such as 'checkDelay' (check interval every hour and send a data point
back to a Google Doc -- which will be discussed), ''evapDelay'
(evaporation compensation interval every four hours - or however long the
user defines this preset), etc. Commenting has been provided in the code
as to what this processes actually do.

void setup() has the pinning information for Ethernet and the air pump.

void loop() has the logic behind time intervals for data collection,
pumping in nutrients, and lighting. 

The next section compensates for evaporation.

The next section is the pinning and logic for the night cycle. So,
when the light turns off for 12 hours, then the pump will remain
on 24/7. 

The next section is for when the system does a light data reading.
The pump will stop for a small increment of time, collect data,
and then start back up. This is so that the bubbling doesn't
interfer with the readings. 

The next three sections sends data to a Google Doc. 


That is all.