This controller is for my swimming pool.
This is about the 5th version/rewrite of the code. It’s the first project I started on about 2009...and actually what introduced me to the wonderful world of Arduino.
It’s a mess, I’ve borrowed code from all edges of the internet (thanks everyone!), its likely full of bugs and highly un-optimised....but it works well enough ;-)

Operational Functions:
Solar pool heating - this measures the roof and pool water temperature and if it’s swimming season and its hot enough then it will switch on the pumps and heat the pool
The controller can also monitor the pH level and run a peristaltic pump to add HCL to the pool.
It has a level of intelligence on when to operate the pumps and chlorinator so as to save power. During summer months the filter needs to operate for approx. 8hrs a day
but in winter, just 4hrs is enough. So during summer, if the pool runs for 2hrs to heat the pool during the day, then it will wait until off peak power time begins, and run the remaining 6hrs then.
During winter, it will only run the pumps during off peak hours.
There is a button on the controller to allow the pump to be stopped/started if required - to backwash for instance.

This version talks MQTT allowing it to be controlled from Openhab. It reports back the status, temperatures, pH level, pump runtimes as well as being able to be controlled via MQTT.
Control functions are pump start/stop (like the button allows), set the desired water temperature, how long in minutes the acid feeder should run each hour (as needed to keep pH in check)
Whilst the Arduino has an accurate DS1307 RTC, it will also pull the time from Openhab - this function was initially only so as to adjust for when daylight savings started/stopped...but I sync it more frequently now.
There is a 20x4 LCD display to show Time, Status, Roof/Water temps, pH as well as runtimes for the filter pump and acid pump.
For communications to MQTT, I’m now using the excellent MySensors system ( mysensors.org ), I was using an ethernet Arduino and an old Wi-Fi router in bridge mode, but the bridge was unreliable.
MySensors has been rock solid on this for 18months or so. There are references to Vera - where I was using MySensors to interface it to Vera, but no more...now MQTT and Openhab.

Hardware:
Arduino Nano running on Optiboot - at some point I was reallllly tight for flash!
This sits on a Nano IO shield - makes radio and I/O connections really easy.
There is a RTC and LCD on the I2C bus.
Manual mode button
It’s got a piezo beeper to alert when it’s in manual mode - so I don’t forget.
It’s got 5 SSR relays - filter pump, chlorinator, solar pump, acid pump and one for the pool lights.

