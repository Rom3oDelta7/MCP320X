/*

Read the values of all MCP ports (channel by channel) and display raw values and voltage conversions

Select the appropriate defines for your installation

Uses default single-ended mode

On an Uno, the reported SPI operation duration is ~ 200usec. For PIN mode, it is ~1520usec. In addition,
there is less sampling time when using PIN mode, so the data is slightly less accurate. Therefore, SPI mode is
strongly encouraged whenever possible.

   Copyright 2016 Rom3oDelta7
   This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.

*/

#define USESPIMODE					// comment out for direct pin mode
#define REF_VOLTAGE		5.0			// set to voltage on VREF MCP pin
#define MCP_PORTS		4			// set to 8 for MCP3208
#define CSPIN  			10    		// Selection Pin (CS)

#include "MCP320x.h"

#if defined (USESPIMODE)
#include <SPI.h>

MCP320x MCP(CSPIN);             					// Constructor for (faster) SPI mode

#else


MCP320x MCP(CSPIN, MOSI, MISO, SCK) ;				// Constructor for manual pin manipulation

#endif


uint16_t ADCValues[MCP_PORTS];                		// array to hold our results

void setup()
{
	Serial.begin(9600);
	while(!Serial);
#if defined(USESPIMODE)
	Serial.println(F("MCP is in SPI mode"));
#else
	Serial.println(F("MCP is in PIN mode"));
#endif
	Serial.println(F("Test using readChannel()\n"));
}

void loop()
{
	uint32_t delta, time_in, time_out;
	
	time_in = micros();
	for ( uint8_t i = 0; i < MCP_PORTS; i++ ) {
		ADCValues[i] = MCP.readChannel(i);
	}
	time_out = micros();
  
	for ( uint8_t i = 0; i < MCP_PORTS; i++ ) {
		Serial.print("CH"); 
		Serial.print(i);     
		Serial.print(": ");         
		Serial.print(ADCValues[i]);    
		Serial.print("\t\t");
		Serial.print(MCP.rawToVoltage(REF_VOLTAGE, ADCValues[i]));  
		Serial.println("V");   
	}
	Serial.print("Operation duration: "); 
	delta = time_out - time_in;	
	Serial.print(delta);    
	Serial.println(" usec");

	delay (2000);

	Serial.println();
	Serial.println();
}


