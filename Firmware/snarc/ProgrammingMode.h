#ifndef PROGRAMMING_MODE_H_
#define PROGRAMMING_MODE_H_

#include "configuration.h"
#include "utils.h"

// Start function declarations
void printProgrammingModeHelp ();
// End function declarations

/**
 * Detects if serial data is entered within 3 seconds. If so enter programming mode.
 * Keeps looping in this method till the user exists programming mode.
 */
void programmingMode ()
{
	clearSerial();

	#ifdef DEBUG
	bool programmingMode = true; // DEBUG: Always enter programming mode
	#else
	Serial.println(F("Press ENTER within 3 seconds to enter programming mode"));
	serialListenForInput(1, 3000, false, true, true);
	bool programmingMode = serial_recieve_index > 0;
	#endif

	if (programmingMode)
	{
		Serial.println(F("Entered programming mode..."));
		flashLed(RED, 200);
		blinkLed(GREEN);
		printProgrammingModeHelp();
	  	clearSerial();
	}
	while (programmingMode)
	{
		manageLeds();
		serialListenForInput(1, 50, false, false, true);
		if (serial_recieve_index > 0)
		{
			flashLed(RED, 200);
			switch (serial_recieve_data[0])
			{
			case 'r':
				Serial.println(F("Read EEPROM list"));
				break;
			case 'x':
				Serial.println(F("Exiting programming mode"));
				programmingMode = false;
				break;
			default:
				Serial.println(F("Unknown input"));
				break;
			}
			if (programmingMode)
			{
				// Print help if we are still in programming mode
				printProgrammingModeHelp();
			}
		}
	}
	ledOff(GREEN);
	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		Serial.println(F("E-STOP triggered while in programming mode."));
	}
	#endif
}

void printProgrammingModeHelp ()
{
	Serial.println();
	Serial.println(F("Programming mode:"));
	Serial.println(F("r - read eeprom list"));
	Serial.println(F("n - program new key to EEPROM"));
	Serial.println(F("s - test key code against server"));
	Serial.println(F("d - set device name"));
	#ifdef ETHERNET
	Serial.println(F("m - set/reset MAC address"));
	Serial.println(F("a - set device IP address (ipv4)"));
	#endif
	Serial.println();
	Serial.println(F("x - exit programming mode"));
}

#endif

