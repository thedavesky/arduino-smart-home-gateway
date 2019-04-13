# arduino-smart-home-advanced-gateway
> An advanced MySensors gateway's code that supports RGB strip (with programs and animations), light bulbs, relays, buttons, OneWire, DHT and radio at one time. 

## General info
I use this code in my intelligent room project. I have it uploaded to the arduino micro clone, which is connected to my home server. It monitors humidity and temperature in my room and temperature outside. It creates charts also. It sets all lighting in my room, both RGB strip and LED lightbulbs. I also have additional things connected via the radio. This sketch uses the maximum possibilities of my arduino. This code has very advanced multitasking.

## Compatible with
* Arduino 1.8.8
* openHAB 2.4
* homebridge 0.4.48
* homebridge-openhab2-complete 0.10.1

## Setup
1. Compile and upload arduino-smart-home-advanced-gateway.ino sketch to your arduino.
2. Install and configure openHAB, MySensors binding, JDBC persistence and MapDB persistence.
3. Copy:
arduinogateway.items to /etc/openhab2/items\
arduinogateway.things to /etc/openhab2/things\
home.sitemap to /etc/openhab2/sitemaps\
jdbc.persist, mapdb.persist to /etc/openhab2/persistence\
tempbalcony.rules to /etc/openhab2/rules
4. Change "/dev/ttyACM0" to your arduino's serial port in /etc/openhab2/things/arduinogateway.things.
5. Install homebridge and homebridge-openhab2-complete.
6. Copy config.json to ~/.homebridge.
7. Enjoy

## Features
* Compatible with openHAB and homebridge
* Advanced multitasking
* Stopwatch rollback protect
* Settings
* NRF24L01+ radio support
* EEPROM saving state
* RGB strip support
* RGB fading program
* RGB smooth fade between colors
* Lightbulbs support
* RGB and lightbulbs power on/off fading
* Power supply startup delay for charging capacitor
* Smart RGB and lightbulbs relay state changing
* Monitor relay support
* DHT humidity and temperature sensor support
* DS18B20 temperature sensor support
* Reporting sensors states
* Change states with buttons
* Multifunction button
* Good optimized code

## Status
Project is: _finished_

## Licence
[GNU General Public License v3.0](https://github.com/thedavesky/arduino-smart-home-advanced-gateway/blob/master/LICENSE)

## Pictures
![Back of my desk 1](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic1.jpg)
![Back of my desk 2](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic2.jpg)
![My workplace](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic3.jpg)
![WWW openHAB](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic4.jpg)
![Homekit integration](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic5.jpg)
![openHAB chart](https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/pics/pic6.jpg)

## Contact
Created by Dawid Maliszewski (thedavesky) <dawid@thedavesky.com>
