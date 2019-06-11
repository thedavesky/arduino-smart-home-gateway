# Arduino Smart Home Gateway
![Release](https://img.shields.io/github/release/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
![Repo size](https://img.shields.io/github/repo-size/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
![Language count](https://img.shields.io/github/languages/count/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
![Top language](https://img.shields.io/github/languages/top/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
![Last commit](https://img.shields.io/github/last-commit/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
![License](https://img.shields.io/github/license/thedavesky/arduino-smart-home-gateway.svg?style=flat-square)
> An advanced MySensors gateway code that supports RGB strip (with modes and animations), LED bulbs, relays, buttons, 1-Wire, DHT and radio at one time.

I use this code in my intelligent room project. I have it uploaded to my arduino micro clone, which is connected to my home server. It monitors humidity and temperature in my room and temperature outside. It also creates charts. It sets all lighting in my room, both RGB strip and LED bulbs. It does animations, transitions between colours and states. It has ability to change modes which can automatically change RGB colours. It controls the state of my computer screen and lighting power supply too. I also have other things connected via the radio. This code uses the maximum possibilities of my arduino.

## Installation

### Arduino
Compile and upload **arduino-smart-home-gateway.ino** sketch to arduino module using Arduino IDE

### openHAB
Install and configure plugins on installed openHAB environment:

- Binding:
	- MySensors
- Persistences:
	- JDBC
	- MapDB

Copy files from **openhab-config** folder:

- **arduinogateway.items** to `/etc/openhab2/items`
- **arduinogateway.things** to `/etc/openhab2/things`
- **home.sitemap** to `/etc/openhab2/sitemaps`
- **jdbc.persist** to `/etc/openhab2/sitemaps`
- **mapdb.persist** to `/etc/openhab2/persistence`
- **tempbalcony.rules** to `/etc/openhab2/rules`

Change "**/dev/ttyACM0**" to arduino's serial port in `/etc/openhab2/things/arduinogateway.things`

### Homebridge
Install **homebridge-openhab2-complete** platform on installed homebridge environment.

Copy **config.json** file from **homebridge-config** folder to `~/.homebridge`

Change host and port in **config.json** file to the ones on which openHAB are listening.

## Compatibility
- openHAB2
- openHAB2 Ambilight Screen
- Homebridge
- Homebridge Plugin For openHAB2 - Complete Edition

## Features
- Advanced multitasking
- Stopwatch rollback protect
- Optimized code
- Delays' settings
- NRF24L01+ radio support
- RGB strip support
- RGB fading and static modes
- RGB smooth fading between colours
- RGB fast fading while fast colours changing
- LED bulbs support
- RGB strip and LED bulbs' fading while state changing
- Saving last state in EEPROM
- Delayed RGB strip saving state to economize EEPROM
- Smart lighting relay state changing
- Lighting power supply startup delay to charge capacitor
- Delayed turning off of lighting relay to economize relay
- Screen relay support
- DS18B20 temperature sensor support
- DHT humidity and temperature sensor support
- Sensors states reporting
- Multifunction button support
- Support changing states using buttons

## Status
Project is: **finished**

## Pictures
<div align="center">
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/openhab_panel.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/openhab_panel.jpg" alt="openHAB panel" width="49.76%">
	</a>
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/homekit_panel.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/homekit_panel.jpg" alt="HomeKit panel" width="49.76%">
	</a>
</div>
<div align="center">
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/openhab_chart.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/openhab_chart.jpg" alt="openHAB chart">
	</a>
</div>
<div align="center">
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/electronics_1.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/electronics_1.jpg" alt="Electronics 1" width="49.76%">
	</a>
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/electronics_2.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/electronics_2.jpg" alt="Electronics 2" width="49.76%">
	</a>
</div>
<div align="center">
	<a href="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/workplace.jpg">
		<img src="https://raw.githubusercontent.com/thedavesky/arduino-smart-home-advanced-gateway/assets/images/workplace.jpg" alt="Workplace">
	</a>
</div>

## Author
Copyright ⓒ 2019 Dawid Maliszewski (thedavesky) <dawid@thedavesky.com>

## Licence
This project is licensed under the **GNU General Public License v3.0** - see the [LICENSE](https://github.com/thedavesky/arduino-smart-home-gateway/blob/master/LICENSE) file for details.
