# Arduino Multi-Sensor Example Code

The following example code is provided for the four-sensor equipped Arduino that is designed to provide a starting point for home automation tasks.

## Hardware Notes

### Basic System Components

The basic system is composed of an Arduino Uno equipped with a Datalogger Shield, which provides a real time clock, the ability to save information to an SD card as well, and a convenient area to route wires.

In particular the basic system is composed of the following parts:

- [Arduino Uno Rev 3](https://store.arduino.cc/usa/arduino-uno-rev3)
- [Adafruit Data Logger Shield](https://www.adafruit.com/product/1141)

## Sensor Package

The system is equipped with four sensors; temperature, light, barometric pressure and humidity. In particular the following sensors are used:

- [TMP36 Analog Temperature Sensor](https://www.adafruit.com/product/165)
- [BMP180 Barometric Pressure Sensors](https://learn.sparkfun.com/tutorials/bmp180-barometric-pressure-sensor-hookup-/installing-the-arduino-library)
- [CdS Photoresistor](https://www.adafruit.com/product/161)
- [DHT11 Temperature and Humidity Sensor](http://www.circuitbasics.com/how-to-set-up-the-dht11-humidity-sensor-on-an-arduino/)

## Using the SD card

It should be noted that there may be pin conflicts preventing the SD card read / write functionality from working. I have not tried to use the SD card reader for some time, but if one was interested in doing so the *Light and Temperature Logger* example code provided by Adafruit ( the Datalogger Shield's manufacturer ) should be a good place to start:
[Light and Temperature Logger Example Code](https://github.com/adafruit/Light-and-Temp-logger/blob/master/lighttemplogger.ino)

## Software Dependencies

The follow libraries are requires to use the provided example code:

- [SFE_BMP180 Library](https://github.com/sparkfun/BMP180_Breakout_Arduino_Library/)
- [RTCLib](https://github.com/adafruit/RTClib)

## Using the Example Program

The example program sets up a simple listening system, where the board will continually poll the USB port, waiting for certain strings. There is one choice string for each sensor that will result in the board polling said sensor and returning its current value over the USB port. The strings the program recognizes are as follows:

| Command | Return Value |
| --- | --- |
|"LIGHT" | Light Sensor value is arbitrary luminosity units ( between ~ 0 - 1000 ) |
|"TEMP" | Temperature in degree Fahrenheit |
| "BARO" | Barometric pressure reading in millibar |
| "HUMI" | Humidity level in % humidity |
| "TIME" | Current time in Year/Month/Day Hour:Minute:Second format |

## Docs

The example program is constructed with comments formatted for Doxygen. Pre-complied HTML docs are provided with the repository under the `/docs` folder, and can be viewed on [this repo's GitHub page](https://bejphil.github.io/Arduino-Multi-Sensor/). If you'd like to build the docs yourself simply copy the `*.ino` files and rename them to `*.cpp` files, and run Doxygen against the base directory.
