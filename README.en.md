# adt7310_emulator
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# About ADT7310
A 16-bit temperature sensor developed and sold by [Analog Devices](https://www.analog.com/jp/index.html).

Model number 7310 supports communication by SPI and 7410 supports communication by I2C.

See [Data Sheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7310.pdf) for the basic specification.

# ADT7310 emulator
[Arduino](https://www.arduino.cc/) and a temperature measurement device using Analog Devices' ADT 7310 are assumed.

I implemented an emulator that sends and receives data via UNIX Socket Domain in Linux environment.

This rewrites the configuration register status of the SPI slave by the command byte string sent from the SPI Master.
Temperature is measured every second and sent by socket communication.

# How to use
## Environment
    * Ubuntu Linux 16.04 LTS
    * GCC 4.2.1

## How to use
Compile using Makefile.

The compiled binary is e4adt7310.

e4adt7310 interprets the command byte to be input and makes appropriate behavior.

## Execution
Grant execute privilege by chmod etc.

## Argument
A file descriptor for imitating SPI communication on Linux is required as an argument.

By default "/tmp/var/spi" is used and can be set to a free position by argument.

Example：
```
./e4adt7310 ~/spi00
```

## About debug
In adt7310.h
```
#define PRINT_SOCK_COMM
#define PRINT_SOCK_COMM_DEBUG
#define DEBUG_PRINT
```
is defined.

Use it according to your needs.

# Known ISSUE
- Behavior when 0x00 is transmitted by SPI communication. Difference from register 0x00 accessed can not be detected.By default, it processes it as a sequence of command bytes to read temperature information.
- In 1 SPS mode, measurement is performed once per second, but in the current implementation it is not implemented to execute temp_gen every second. Prepare a counter variable and generate temperature only when it reaches a certain numerical value.

# Special Thunks
The library for the ADT 7310 used for the test environment is [scogswell/ArduinoADT 7310] (https://github.com/scogswell/ArduinoADT7310).


# References
[wsnakのブログ](http://www.wsnak.com/wsnakblog/?p=249)

[秋月電子通商](http://akizukidenshi.com/catalog/g/gM-06708/)

[Analog Devices](https://www.analog.com/jp/index.html)

[scogswell/ArduinoADT7310](https://github.com/scogswell/ArduinoADT7310)

