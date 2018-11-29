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

By default "/tmp/spi" is used and can be set to a free position by argument.

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


# References
[wsnakのブログ](http://www.wsnak.com/wsnakblog/?p=249)

[秋月電子通商](http://akizukidenshi.com/catalog/g/gM-06708/)

[Analog Devices](https://www.analog.com/jp/index.html)

