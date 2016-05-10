/*

Demonstrate MCP configuration interface
Note that ports can be configured even if they don't physically exist
e.g. you can configure all 8 ports even if you are using a 4-port ADC

Select the appropriate defines for your installation

Uses default single-ended mode

   Copyright 2016 Rom3oDelta7
   This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.

*/

#define REF_VOLTAGE		5.0			// set to voltage on VREF MCP pin
#define CSPIN  			10    		// Selection Pin (CS)

#include "MCP320x.h"
#include <SPI.h>

MCP320x MCP(CSPIN);             	// Constructor for SPI mode

void setup()
{
	Serial.begin(9600);
	while(!Serial);
	Serial.println(F("Demonstrate MCP320x config\n"));
}

void printConfig ( const uint8_t chan ) {
	MCPMode mode;

	mode = MCP.getMCPConfig(chan);
	Serial.print("CH"); 
	Serial.print(chan);     
	Serial.print(": ");         
	if ( mode == MCP_SINGLE ) {
		Serial.print("SINGLE ");
	} else {
		Serial.print("DIFFERENTIAL ");
	} 
	Serial.println(" mode");
}

void loop()
{
	MCP.setMCPConfig(MCP_SINGLE, MCP_ALL_PORTS);
	
	Serial.println(F("Setting ALL channels to SINGLE ..."));
	for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) {
		printConfig(i);
	}
	Serial.println();

	/* 
	 now set differential mode
	 in this mode, configuring one channel of a pair will automatically set the other one
	*/
	Serial.println(F("Setting channels 1, 4, and 8 to DIFFERENTIAL ..."));
	MCP.setMCPConfig(MCP_DIFFERENTIAL, 1);
	MCP.setMCPConfig(MCP_DIFFERENTIAL, 4);
	MCP.setMCPConfig(MCP_DIFFERENTIAL, 8);
	
	for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) {
		printConfig(i);
	}
	
	Serial.println();
	
	// try an invalid channel
	if ( !MCP.setMCPConfig(MCP_DIFFERENTIAL, 12) ) {
		Serial.println("Error returned when attempting to configure an invalid channel number");
	}
	
	delay(2000);
	Serial.println();
	Serial.println();
}


