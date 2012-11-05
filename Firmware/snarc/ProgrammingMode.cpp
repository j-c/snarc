#include "ProgrammingMode.h"

#ifdef ENABLE_ESTOP
extern bool lockdown = false;
#endif

/**
 * Detects if serial data is entered within 3 seconds. If so enter programming mode.
 * Keeps looping in this method till the user exists programming mode.
 */
void programmingMode ()
{
        char serial_recieve_data[SERIAL_RECIEVE_BUFFER_LENGTH];
	clearSerial();

	#ifdef DEBUG
	bool programmingMode = true; // DEBUG: Always enter programming mode
	#else
	Serial.println("Press ENTER within 3 seconds to enter programming mode");
	serialListenForInput(1, 3000, false, true, true, serial_recieve_data);
	bool programmingMode = strlen(serial_recieve_data) > 0;
	Serial.print("xxx..");
	Serial.println(strlen(serial_recieve_data) > 0);
	Serial.println(programmingMode);
	#endif

	if (programmingMode)
	{
		Serial.println("Entered programming mode...");
		flashLed(RED, 200);
		blinkLed(GREEN);
		printProgrammingModeHelp();
	  	clearSerial();
	}
	while (programmingMode)
	{
		manageLeds();
		serialListenForInput(1, 50, false, false, false, serial_recieve_data);
		
		if (strlen(serial_recieve_data) > 0)
		{
			flashLed(RED, 200);
			switch (serial_recieve_data[0])
			{
			case 'r':
				Serial.println("Read EEPROM list");
				break;
			case 'x':
				Serial.println("Exiting programming mode");
				programmingMode = false;
				break;
			case 'd':
				//listen_for_device_name();
				break;
#ifdef ETHERNET
			case 'm':
				//listen_for_new_mac_address();
				break;
			case 'v':
				//listen_for_auth_server_ip();
				break;
			case 'a':
				//listen_for_ip_address();
				break;
#endif
			case '!':
				wipeEeprom();
				break;
			default:
				Serial.println("Unknown input");
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
		Serial.println("E-STOP triggered while in programming mode.");
	}
	#endif
}

void printProgrammingModeHelp ()
{
	Serial.println();
	Serial.println("Programming mode:");
	Serial.println("r - read eeprom list");
	Serial.println("n - program new key to EEPROM");
	Serial.println("s - test key code against server");
	Serial.println("d - set device name");
	#ifdef ETHERNET
	Serial.println("m - set/reset MAC address");
	Serial.println("a - set device IP address (ipv4)");
	Serial.println("v - set authentication server IP address (ipv4)");
	#endif
	Serial.println("! - wipe eeprom");
	Serial.println();
	Serial.println("x - exit programming mode");
}

void wipeEeprom()
{
	Serial.print("Wiping EEPROM wiped...");
	for (int i = 0; i < EEPROM_MAX_ADDRESS; i++)
	{
		EEPROM.write(i, 0xff);
	}
	Serial.println(" Done.");
}


#ifdef ETHERNET
/*
Listens for new MAC address over serial or creates random. Warning... here be dragons...
*/
void listen_for_new_mac_address () // blocking operation
{
	Serial.print("Current MAC address is ");
	print_mac_address();
	Serial.println();

	Serial.println("Enter new MAC address 01:23:45:67:89:AB (enter a newline for random MAC address):");
	byte b;
	byte bi = 0; // Index for octetB
	byte index = 0;
	byte acc = 0;
	uint8_t macOctets[6];
	clearSerial();
	while (true)
	{
		if (Serial.available())
		{
			b = Serial.read();
			if (b == 10 || b == 13)
			{
				if (index == 0)
				{
					// Nothing entered, use random MAC
					generate_random_mac_address(macOctets);
					Serial.print("New MAC: ");
					print_mac_address();
					Serial.println();
				}
				else if (index == 6)
				{
					// nothing. Should be valid.
				}
				else
				{
					Serial.println("Invalid mac address!");
				}
				break;
			}
			else if ((b >= 48 && b <= 58) || (b >= 97 && b <= 102) || (b >= 65 && b <= 70)) // 0-9: || a-z || A-Z
			{
				if (b >= 48 && b <= 57) // 0-9
				{
					acc *= 16;
					acc += (b - 48);
					bi++;
				}
				else if (b != 58) // (a-f || A-F)
				{
					if (b >= 97) // convert to uppercase
					{
						b -= 32;
					}
					acc *= 16;
					acc += (b - 55);
					bi++;
				}
				if (bi == 2 && index == 5)
				{
					b = 58; // Force b = 58 so that the final octet is added
				}
				else if (bi > 2)
				{
					// Invalid
					Serial.println("Invalid mac address!");
					break;
				}
				
				if (b == 58) // :
				{
					if (bi == 2)
					{
						// Set octet & reset counters
						macOctets[index++] = acc;
						acc = 0;
						bi = 0;
						if (index == 6)
						{
							// Done. Save
							save_mac_address(macOctets);
							print_mac_address();
							Serial.println();
							break;
						}
					}
					else if (bi > 2)
					{
						Serial.println("Invalid mac address!");
					}
				}
			}
			else
			{
				// Invalid
				Serial.println("Invalid mac address!");
				break;
			}
		}
	}
	clearSerial();
}
/**
 * End: MAC Address
 */
 
#endif
