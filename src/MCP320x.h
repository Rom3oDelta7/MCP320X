
/*
 Driver library for SPI-based MCP3204/MCP3208 12-bit ADC
 Based on the following sources:
 http://arduino.cc/forum/index.php?topic=53082.0 (see  A. Hinkel library from April 2012-04-26)
 http://arduino.cc/playground/Code/MCP3208	
 A. Hinkel library from April 2012-04-26
 
 Rewritten by Rom3oDelta7 2016-4-5 to conform to new SPI library in IDE 1.6.5 and to allow both 5V and 3.3V operation.
 Types and functions have been redefined so this library is not directly compatible with the previous versions.
 
 For reference, see the Microchip MCP3204/3208 datasheet at http://ww1.microchip.com/downloads/en/DeviceDoc/21298D.pdf
 
 This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
*/


#ifndef MCP320x_H
#define MCP320x_H

#if defined(ARDUINO) && (ARDUINO >= 10605)
#include "Arduino.h"
#else
#error This library requires Arduino IDE 1.6.5 or above
#endif

#define	MCP_MAX_PORTS		8
#define MCP_ALL_PORTS		0
#define MCP_CHANNEL_ERROR	0xFFFF					// return code for channel out of range (not used for MODE)

typedef enum { MCP_SINGLE, MCP_DIFFERENTIAL, MCP_RANGE_ERROR } MCPMode;
typedef	enum { MCP_PIN_MODE, MCP_SPI_MODE } DriverMode;


class MCP320x {
  public:
	// constructors 
    MCP320x(uint8_t CS, uint8_t DIN, uint8_t DOUT, uint8_t CLK);						// pin mode (~7-8x SLOWER than SPI mode)
    MCP320x(uint8_t CS);																// SPI mode
	
	// user functions
    uint16_t	readChannel(uint8_t channel);											// read the selected MCP channel
    bool 		readAllChannels(uint16_t channelValue[], uint8_t channelCount);			// read all channels up to count into given array
    MCPMode 	getMCPConfig(uint8_t channel);											// get SGL/DIFF config for given channel
    bool 		setMCPConfig(MCPMode mode, uint8_t channel);							// set channel SGL/DIFF config for a channel (MCP_ALL_PORTS to set all channels)
    float 		rawToVoltage(float VREF, uint16_t ADCRawValue);							// convert ADC raw value to volts using given reference voltage (MCP VREF pin)

  private:
    MCPMode		_DeviceMode[MCP_MAX_PORTS];			// MCP sampling mode configuration
    uint8_t		_CS;								// chip select
    uint8_t		_DOUT;								// MCP Dout (MISO)
    uint8_t		_DIN;								// MCP Din (MOSI)
    uint8_t		_CLK;								// clock pin
    DriverMode	_ioMode;
    uint16_t 	_read_SPI(uint8_t channel);			// private read functions based on mode
    uint16_t 	_read_pin(uint8_t channel);
    void 		_clockTick(uint8_t n);				// used for pin mode only
};

#endif
