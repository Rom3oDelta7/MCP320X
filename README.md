# MCP3204/8 Arduino Library
MCP320x is an Arduino library (for IDE version 1.6.5 and later) that provides an SPI interface to the Microchip 3204 and 3208 12-bit analog to digital converter (ADC) [datasheet].
This is a rewrite of an older, similar library (see http://arduino.cc/forum/index.php?topic=53082.0 and http://arduino.cc/playground/Code/MCP320).

This is a rewrite to conform to new SPI library SDK in IDE 1.6.5 and to allow both 5V and 3.3V operation.
Types and functions have been redefined so this library is not directly compatible with the previous versions.
However, the main concept in the older library of allowing for both SPI as well as direct pin manipulation modes is preserved.
While the pin manipulation mode may be useful if SPI is not an option, it is 7-8 times slower than the SPI interface.

# Programming Interface
## Constructors
```C++
MCP320x(uint8_t CS, uint8_t DIN, uint8_t DOUT, uint8_t CLK);
```
Pin mode constructor.

|Parameter|Purpose|
|---------|------------|
CS|chip select
DIN|MCP data in
DOUT|MCP data out
CLK|clock

```C++
MCP320x(uint8_t CS);
```
SPI mode constructor (uses standard SPI pins (depends on your MCU))

|Parameter|Purpose|
|---------|------------|
CS|chip select

## Fucntions



## Copyright Notice

Copyright 2016 Rob Redford
This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
To view a copy of this license, visit [BY-NC_SA].


[BY-NC_SA]: http://creativecommons.org/licenses/by-nc-sa/4.0
[datasheet]: http://ww1.microchip.com/downloads/en/DeviceDoc/21298c.pdf