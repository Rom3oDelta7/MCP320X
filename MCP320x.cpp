/*
 Driver library for SPI-based MCP3204/MCP3208 12-bit ADC
 Based on the following sources:
 http://arduino.cc/forum/index.php?topic=53082.0 (see  A. Hinkel library from April 2012-04-26)
 http://arduino.cc/playground/Code/MCP3208	

 
 Rewritten by Rom3oDelta7 2016-4-5 to conform to new SPI library in IDE 1.6.5 and to allow both 5V and 3.3V operation.
 Types and functions have been redefined so this library is not directly compatible with the previous versions.
 
 For reference, see the Microchip MCP3204/3208 datasheet at http://ww1.microchip.com/downloads/en/DeviceDoc/21298D.pdf
 
 
 This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
*/
 

#if defined(ARDUINO) && (ARDUINO >= 10605)
#include "Arduino.h"
#else
#error This library requires Arduino IDE 1.6.5 or above
#endif

#include "MCP320x.h"
#include <SPI.h>

#define SPI_ADC_RESOLUTION		4096.0						// 12-bit MCP320x ADC resolution
#define MCP_CLOCK				1000000UL					// 1MHz clock rate - for compatibility, use lower rate (for 3.3V systems)

/*
 direct PIN mode, not using SPI library
 
 On an Uno, SPI mode is 7-8x faster then PIN mode. In addition,
 there is less sampling time when using PIN mode, so the data is slightly less accurate. Therefore, SPI mode is
 strongly encouraged whenever possible.
 
*/
MCP320x::MCP320x ( const uint8_t CS, const uint8_t DIN, const uint8_t DOUT, const uint8_t CLK ) {
	_CS      = CS;
	_DIN     = DIN;
	_DOUT    = DOUT;
	_CLK     = CLK;

	_ioMode = MCP_PIN_MODE;

	for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) { 
		// set default sampling mode
		_DeviceMode[i] = MCP_SINGLE;
	}

	// definitions are relative to AVR
	pinMode(_CS, OUTPUT); 
	pinMode(_CLK, OUTPUT); 
	// definitions relative to the MCP
	pinMode(_DOUT, INPUT); 
	pinMode(_DIN, OUTPUT); 

	// set initial states for the control lines
	digitalWrite(_CS, HIGH); 								// disable MCP (active low)
	digitalWrite(_DIN, LOW); 
	digitalWrite(_CLK, LOW);								// device latches data on rising edge, so lock must remain low when idle
}

/*
 faster mode using SPI library
*/
MCP320x::MCP320x ( const uint8_t CS ) {
	_CS   = CS;
	_ioMode = MCP_SPI_MODE;

	for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) { 
		// set default sampling mode
		_DeviceMode[i] = MCP_SINGLE;
	}

	pinMode(_CS, OUTPUT);
	digitalWrite(_CS, HIGH);								// disable MCP
	SPI.begin();
}

/*
 returns the sampling config (single/differential) of the given channel or MCP_RANGE_ERROR if channel is out of range
*/
MCPMode MCP320x::getMCPConfig ( const uint8_t channel ) {
	if ( channel < MCP_MAX_PORTS ) {
		return _DeviceMode[channel];
	} else {
		return MCP_RANGE_ERROR;
	}
}

/*
 Set the sampling config (single/differential) of the given channel or ALL channels if channel == MCP_ALL_PORTS
 For DIFFERENTIAL mode, the channels operate in pairs. Setting either one will also set the other.
 Returns true if successful, else false
 
*/
bool MCP320x::setMCPConfig ( const MCPMode mode, const uint8_t channel ) {
	bool retval;
	
	if ( channel == MCP_ALL_PORTS ) {
		for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) {
			_DeviceMode[i] = mode;
		}
		retval = true;
	} else if ( channel < MCP_MAX_PORTS ) {
		if ( mode == MCP_SINGLE ) {
			_DeviceMode[channel] = mode;
		} else {
			// always pair up differential channels by inverting the LSbit to get the other channel number (2 -> 3, 3 -> 2, etc)
			uint8_t	altChannel = (channel & B110) | (channel ^ B001);
			
			_DeviceMode[channel] = mode;
			_DeviceMode[altChannel] = mode;
		}
		retval = true;
	} else {
		retval = false;
	}
	return retval;
}

/*
 read all MCP channels (based on given channel count)
 There is no way in software to determine if this is a 4- or 8-port chip, so leave it to the programmer to manage
*/
bool MCP320x::readAllChannels ( uint16_t channelValue[], const uint8_t channelCount ) {

	if ( channelCount > MCP_MAX_PORTS ) {
		return false;
	}
	if ( _ioMode == MCP_SPI_MODE ) {
		for ( uint8_t i = 0; i < channelCount; i++ ) {
			channelValue[i] = _read_SPI(i);
		}
	} else {
		for ( uint8_t i = 0; i < channelCount; i++ ) {
			channelValue[i] = _read_pin(i);
		}
	}
	return true;
} 

/*
 read an individual channel, using the currently set mode
*/ 
uint16_t MCP320x::readChannel ( const uint8_t channel ) {
	if ( channel >= MCP_MAX_PORTS ) {
		return MCP_CHANNEL_ERROR;
	} else if (_ioMode == MCP_SPI_MODE ) {
    	return _read_SPI(channel);
	} else {
    	return _read_pin(channel);
	}
}

/*
 Caluculate the voltage given the raw (digitized) value of the channel
*/
float MCP320x::rawToVoltage ( const float VREF, const uint16_t ADCRawValue) {

    return (float)(ADCRawValue * (VREF/SPI_ADC_RESOLUTION));
}

/*
 For timing info and bit pattern, see Figure 6-1 in the Microchip MCP3204/8 Data Sheet
 Note that we are using the conservative clock value - 1MHz for 2.7V, 2MHz for 5V, so assume the lower value to be safe
*/
uint16_t MCP320x::_read_SPI ( const uint8_t channel ) {
	uint8_t		msb, lsb;
	uint16_t 	commandWord;
	uint16_t	digitalValue;
	
	/*
	 Build the following command pattern that will be sent as 3 bytes in separate transactions:
	 <5 leading 0s><start bit><sgl/diff><D2> <D1><D0><6 0s> <8 zeros>
	 byte    1                   2                    3
	 
	 And we expect to get:
	 <8 don't care bits> <<3 don't care bits>0<B11-B8>> <B7-B0>
	         1                     2                       3
	*/ 
	if ( _DeviceMode[channel] == MCP_SINGLE ) {
		commandWord = (_BV(10) | _BV(9));									// start bit(10) + single-ended(9)
	} else {
		commandWord = _BV(10);												// start bit(10) + double(9 is 0)
	}
	commandWord |= (uint16_t)(channel << 6);								// 3-bit channel number D2-D0
	
	// now send this command word as 3 bytes: MSB, LSB, then 0 (3rd is really a don't care)
	SPI.beginTransaction(SPISettings(MCP_CLOCK, MSBFIRST, SPI_MODE0));		// use highest clock allowed for 3.3V operation
	digitalWrite(_CS, LOW);			 										// Select ADC
	SPI.transfer(highByte(commandWord));									// send MSB, get byte 1: we ignore the returned value - it's garbage
	// keep msb & lsb in separate variables for debugging the SPI interface
	msb = SPI.transfer(lowByte(commandWord));								// send LSB, get byte 2: ???0<B11-B8>
	digitalValue = (uint16_t)((msb & 0x0F) << 8);							// mask off unused top 4 bits and move to MSB
	lsb = SPI.transfer(0);													// send don't care byte: get <B7-B0>
	digitalValue |= (uint16_t)lsb;											// add the LSB to the returned value
	digitalWrite(_CS, HIGH);			 									// De-select ADC
	SPI.endTransaction();
	
	return digitalValue;
}

 
/*
 Use direct pin manipulation to read the data without SPI manually, a bit at a time.
 May be useful for situations where the SPI library cannot be used, we need to use different pins, etc.
*/

uint16_t MCP320x::_read_pin ( const uint8_t channel ) {
	uint16_t	ADCvalue = 0;
	uint8_t 	commandByte;


	/*
	 Build the following command pattern that will be sent as 5 bits in separate transactions:
	 <start bit><sgl/diff><D2><D1><D0>
	 
	 And we expect to get:
	 <2 null bits><B11-B0>
	*/ 
	if ( _DeviceMode[channel] == MCP_SINGLE ) {
		commandByte = (_BV(7) | _BV(6));									// start bit + single-ended
	} else {
		commandByte = _BV(7);												// start bit + double(0)
	}
	commandByte |= (channel << 3);											// 3-bit channel number D2-D0

	digitalWrite(_CS, LOW); 												// Select ADC
	for (uint8_t i = 7; i >= 3; i-- ){										// individually send top 5 bits
		digitalWrite(_DIN, (commandByte & (1 << i)));						// select the bit and set line high or low
		_clockTick(1);    													// cycle clock 1x
	}
	_clockTick(2);															// 1 clock for sampling, 1 clock for null bit (MISO)

	for ( int8_t i = 11; i >= 0; i-- ){  									// read channel value from MCP, bitwise, highest bit first, 12 bits
		ADCvalue |= digitalRead(_DOUT) << i;    
		_clockTick(1);
	}

	digitalWrite(_CS, HIGH); 												// disable MCP. Note that the CLK remains low

	return ADCvalue;

}
 

/*
 Manually manipulate the CLK signal
 Note that the clock is assumed to be low to start with as the MCP latches data on the rising edge
*/
void MCP320x::_clockTick ( const uint8_t ticks ) {   

  for (uint8_t i = 0; i < ticks; i++ ){			
    digitalWrite(_CLK, HIGH);    			
    digitalWrite(_CLK, LOW);
  }
}


