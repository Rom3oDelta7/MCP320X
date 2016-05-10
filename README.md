# MCP3204/8 Arduino Library
MCP320x is an Arduino library (for IDE version 1.6.5 and later) that provides an SPI interface to the [Microchip 3204 and 3208 12-bit analog to digital converter] (ADC).
This is a rewrite of an older, similar library (see http://arduino.cc/forum/index.php?topic=53082.0 and http://arduino.cc/playground/Code/MCP320).

This new version conforms to SPI library SDK in IDE 1.6.5 and allows both 5V and 3.3V operation (voltage support is transparent and does not need to be specified by the user).
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
|----|-------|
CS|chip select
DIN|MCP data in
DOUT|MCP data out
CLK|clock

```C++
MCP320x(uint8_t CS);
```
SPI mode constructor (uses standard SPI pins MISO, MOSI, and SCK (pin numbers depend on your MCU))

|Parameter|Purpose|
|---|-------|
CS|chip select

## Functions
```C++
uint16_t readChannel(uint8_t channel);
```
Read the selected MCP _channel_ and return the value. Returns **MCP_CHANNEL_ERROR** for _channel_ values out of range.

```C++
bool readAllChannels(uint16_t channelValue[], uint8_t channelCount);
```
Read all channels up to _channelCount_ into the given array (_channelValue_). Returns **true** if successful, else **false**.

```C++
MCPMode getMCPConfig(uint8_t channel);
```
Get configuration (single-ended(**MCP_SINGLE**) or differential(**MCP_DIFFERENTIAL**)) for the given _channel_. Returns **MCP_RANGE_ERROR** for values out of range.

```C++
bool setMCPConfig(MCPMode mode, uint8_t channel);
```
Sets _channel_ configuration to single-ended(**MCP_SINGLE**) or differential(**MCP_DIFFERENTIAL**). Use **MCP_ALL_PORTS** as the channel parameter to set all channels.
Returns **true** if successful, else **false**.

```C++
float rawToVoltage(float VREF, uint16_t ADCRawValue);	
```
Convert output from `readChannel()`, _ADCRawValue_, to volts using given reference voltage _VREF_. This must be the same as the voltage supplied to the MCP VREF pin.

## Examples
1. _MCP320x_array.ino_: Read the values of all ADC ports (with a single function call) and display raw values and voltage conversions.
2. _MCP320x_channel.ino_: Read the values of all MCP ports (channel by channel) and display raw values and voltage conversions.
3. _MCP320x_config.ino_: Demonstrate MCP configuration interface.


## Copyright Notice

Copyright 2016 Rob Redford
This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
To view a copy of this license, visit [BY-NC-SA].


[BY-NC-SA]: http://creativecommons.org/licenses/by-nc-sa/4.0
[Microchip 3204 and 3208 12-bit analog to digital converter]: http://ww1.microchip.com/downloads/en/DeviceDoc/21298c.pdf