// https://github.com/j-c/snarc

#include "MemoryFree.h"
#include "configuration.h"

#ifdef ENABLE_ESTOP
	bool lockdown = false;
#endif

#include <EEPROM.h>

#ifdef ETHERNET
	#include <SPI.h> //needed by Ethernet.h
	#include <Ethernet.h>
#endif

#include <SoftwareSerial.h>


#include "utils.h"
#include "led.h"
#include "ProgrammingMode.h"

#include "IRFIDReader.h"
#include "IAuthenticator.h"
#include "IActuator.h"

// Single RED FLASH with BLINKING GREEN programming mode. Will FLASH RED with each command.
// Single RED FLASH during operation means card ID does not have access.
// Single GREEN FLASH during operation means card ID has access.
// SOLID RED with BLINKING GREEN means E-Stop condition.


char device_name[DEVICE_NAME_MAX_LENGTH + 1]; // +1 for terminating char



/* USAGE: be sure to revise this script for each peice of hardware:
 * change the IP address that it should be assigned to ( default is 192.168.7.7 ) 
 * make sure the "server" it will authenticate to is correct ( default is 192.168.7.77 ) 
 * make sure the URL that is being used for your "auth server" is correct.  ( default is /auth.php? ) 
 * make sure you have it wired-up correctly! Code currently assumes the RFID reader is on Software Serial on RX/TX pins  D15 and D16 ( also sometimes called A1 and A2 )
*/

#ifdef ETHERNET
// network settings:
byte mac[] = {0, 0 ,0 ,0 ,0 ,0};

byte clientIpBytes[] = {0, 0, 0, 0};
IPAddress ip(0, 0, 0, 0);

byte authServerIpBytes[] = {0, 0, 0, 0};
//IPAddress server(0, 0, 0, 0); // destinaton end-point is "auth server" internal IP.
EthernetClient client;

bool ethernetInit = false;

#else
#endif



void setup()
{
	// Needed for random MAC address
	randomSeed(analogRead(A1));

	Serial.begin(SNARC_BAUD_RATE);

	Serial.println();

	programmingMode();

	// Display basic config details
	Serial.println();

	load_device_name();
	Serial.print(F("Name: "));
	print_device_name();
	Serial.println();

	#ifdef ETHERNET
	Serial.println(F("Starting ethernet..."));
	initEthernet();
	#endif

	// Actuator init
	pinMode(ACTUATOR_PIN, OUTPUT);
	digitalWrite(ACTUATOR_PIN, LOW);

	#ifdef ENABLE_ESTOP
	eStopInit();
	#endif

	Serial.println(F("Started"));

	// Init status LEDs
	pinMode(RED_LED_PIN, OUTPUT);
	digitalWrite(RED_LED_PIN, LOW);
	pinMode(GREEN_LED_PIN, OUTPUT);
	digitalWrite(GREEN_LED_PIN, LOW);

	delay(3000);
	authenticator_init();

	Serial.println(F("Entering operation mode! "));
}


void loop ()
{
	manageLeds();

	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		eStopLockdown();
	}
	#endif

	
	// Check to see if card is available
	if (reader_available())
	{
		#ifdef DEBUG
				Serial.print("Memory free: ");
				Serial.println(freeMemory());
		#endif
		unsigned long id = reader_read();
		if (authenticator_authenticate(id, device_name))
		{
			// Success
			flashLed(GREEN, 2000);
			manageLeds();
			Serial.println(F("ID has access"));
			actuator_on();
		}
		else
		{
			// Fail
			flashLed(RED, 1000);
			actuator_off();
			Serial.println(F("ID does not have access"));
			// TODO: Implement timeout and turn off if no valid card found
		}
		// TODO: wait/sleep for a while before attempting read again?
	}

}


#ifdef ETHERNET
void initEthernet ()
{
	ethernetInit = true; // This is actually quite bad, but currently ethernet init status is set to true as a quick fix. Most failure modes seem to lockup the Arduino anyway.
	load_ip_address();
	load_auth_server_ip();
	get_mac_address();
	ip = IPAddress(clientIpBytes[0], clientIpBytes[1], clientIpBytes[2], clientIpBytes[3]);

	bool useDhcp = (clientIpBytes[0] == 0 && clientIpBytes[1] == 0 && clientIpBytes[2] == 0 && clientIpBytes[3] == 0) || (clientIpBytes[0] == 0xff && clientIpBytes[1] == 0xff && clientIpBytes[2] == 0xff && clientIpBytes[3] == 0xff);
	if (useDhcp)
	{
		Serial.println("Attempting to aquire IP via DHCP...");
		if (Ethernet.begin(mac) == 0)
		{
			Serial.println(F("Failed to configure Ethernet using DHCP"));
			ethernetInit = false;
			// TODO: Do something aout this error
		}
	}
	else
	{
		Ethernet.begin(mac, ip);
	}

	Serial.print(F("                      IP: "));
	print_ip_address();
	Serial.println();

	Serial.print(F("                     MAC: "));
	print_mac_address();
	Serial.println();

	Serial.print(F("Authentication server IP: "));
	print_auth_server_ip();
	Serial.println();
}


/**
 * Start: Mac Address
 */
void print_mac_address ()
{
	if (mac[0] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[0], HEX);
	Serial.print(':');
	if (mac[1] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[1], HEX);
	Serial.print(':');
	if (mac[2] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[2], HEX);
	Serial.print(':');
	if (mac[3] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[3], HEX);
	Serial.print(':');
	if (mac[4] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[4], HEX);
	Serial.print(':');
	if (mac[5] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[5], HEX);
}

void save_mac_address ()
{
	for (byte i = 0; i < 6; i++)
	{
		EEPROM.write(MAC_START_ADDRESS + i, mac[i]);
	}
}

void generate_random_mac_address ()
{
	set_mac_address(random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255));
}

void get_mac_address ()
{
	bool hasNeverBeenSaved = true;
	for (byte i = 0; i < 6; i++)
	{
		mac[i] = EEPROM.read(MAC_START_ADDRESS + i);
		if (mac[i] != 255)  // If a EEPROM cell has never been written to before, it returns 255 (accordoing to Arduino doco)
		{
			hasNeverBeenSaved = false;
		}
	}
	if (hasNeverBeenSaved)
	{
		Serial.print(F("Generating random MAC address: "));
		generate_random_mac_address();
		print_mac_address();
		Serial.println();
	}
}

void set_mac_address (byte octet0, byte octet1, byte octet2, byte octet3, byte octet4, byte octet5)
{
	mac[0] = octet0;
	mac[1] = octet1;
	mac[2] = octet2;
	mac[3] = octet3;
	mac[4] = octet4;
	mac[5] = octet5;
	save_mac_address();
}

/*
Listens for new MAC address over serial or creates random. Warning... here be dragons...
*/
void listen_for_new_mac_address () // blocking operation
{
	
	Serial.print(F("Current MAC address is "));
	print_mac_address();
	Serial.println();

	Serial.println(F("Enter new MAC address 01:23:45:67:89:AB (enter a newline for random MAC address):"));
	byte b;
	byte bi = 0; // Index for octetB
	byte index = 0;
	byte acc = 0;
	byte octetB[] = {0, 0};
	char macOctets[] = {0, 0, 0, 0, 0, 0};
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
					generate_random_mac_address();
					Serial.print(F("New MAC: "));
					print_mac_address();
					Serial.println();
				}
				else if (index == 6)
				{
					// nothing. Should be valid.
				}
				else
				{
					Serial.println(F("Invalid mac address!"));
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
					Serial.println(F("Invalid mac address!"));
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
							set_mac_address(macOctets[0], macOctets[1], macOctets[2], macOctets[3], macOctets[4], macOctets[5]);
							print_mac_address();
							Serial.println();
							break;
						}
					}
					else if (bi > 2)
					{
						Serial.println(F("Invalid mac address!"));
					}
				}
			}
			else
			{
				// Invalid
				Serial.println(F("Invalid mac address!"));
				break;
			}
		}
	}
	clearSerial();
}
/**
 * End: MAC Address
 */

/**
 * Start: IP Address
 */
void print_ip_address()
{
	Serial.print(Ethernet.localIP());
	/*
	Serial.print(clientIpBytes[0], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[1], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[2], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[3], DEC);
	*/
	if (clientIpBytes[0] == 0 && clientIpBytes[1] == 0 && clientIpBytes[2] == 0 && clientIpBytes[3] == 0)
	{
		Serial.print(" (via DHCP)");
	}

}

void save_ip_address()
{
	Serial.println("Saving IP Address...");
	for (byte i = 0; i < 4; i++)
	{
		//Serial.println(EEPROM.read(IP_ADDRESS + i));
		//Serial.println(clientIpBytes[i]);
		EEPROM.write(IP_ADDRESS + i, clientIpBytes[i]);
		//Serial.println(EEPROM.read(IP_ADDRESS + i));
		//Serial.println();
	}
}

void load_ip_address ()
{
	Serial.println("Loading IP Address from EEPROM (if available)...");
	for (byte i = 0; i < 4; i++)
	{
		clientIpBytes[i] = EEPROM.read(IP_ADDRESS + i);
	}
}

void listen_for_ip_address()
{
	Serial.print(F("Current ip address: "));
	print_ip_address();
	Serial.println();
	Serial.println(ip);

	Serial.println(F("Enter new ip address (press enter to use DHCP):"));
	
	serial_recieve_index = 0;
	clearSerial();

	bool keepReading = true;
	while (keepReading)
	{
		while (Serial.available())
		{
			if (Serial.peek() == 13 || Serial.peek() == 10)
			{
				// new line. End entry
				keepReading = false;
				break;
			}
			serial_recieve_data[serial_recieve_index++] = Serial.read();
			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
			{
				// max length. End entry
				keepReading = false;
				break;
			}
		}
	}
	clearSerial();

	if (serial_recieve_index == 0)
	{
		// Empty, use DHCP
		clientIpBytes[0] = 0;
		clientIpBytes[1] = 0;
		clientIpBytes[2] = 0;
		clientIpBytes[3] = 0;
		save_ip_address();
		Serial.print(F("Using DHCP."));
	}
	else
	{
		boolean validInput = true;
		int octet = 0;
		byte newIp[] = {0, 0, 0, 0};
		byte newIpIndex = 0;
		for (byte i = 0; i < serial_recieve_index; i++)
		{
			if (serial_recieve_data[i] >= 48 && serial_recieve_data[i] <= 57) // 0-9
			{
				octet = octet * 10 + (serial_recieve_data[i] - 48);
				if (octet > 255)
				{
					validInput = false;
					break;
				}
			}
			else if ((char)serial_recieve_data[i] == '.') // TODO: replace with ASCII code for '.'
			{
				newIp[newIpIndex++] = octet;
				octet = 0;
			}
			else if (serial_recieve_data[i] == 10 || serial_recieve_data[i] == 13) // newline
			{
				if (octet != 0 || newIpIndex != 0) // no input parsed yet
				{
					validInput = false;
				}
				else
				{
					newIp[newIpIndex++] = octet;
				}
				break;
			}
			else // invalid char
			{
				validInput = false;
				break;
			}
		}

		if (validInput)
		{
			clientIpBytes[0] = newIp[0];
			clientIpBytes[1] = newIp[1];
			clientIpBytes[2] = newIp[2];
			clientIpBytes[3] = newIp[3];
			save_ip_address();
			Serial.println(F(" Please reset SNARC for changes to take effect."));
		}
		else
		{
			Serial.println(F("Invalid input. IP address not changed."));
		}
	}
}
#endif
/**
 * End: IP address
 */


/**
 * Start: Device name
 */
void print_device_name()
{
	Serial.print(device_name);
}

void save_device_name()
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		EEPROM.write(DEVICE_NAME_ADDRESS + i, (byte)device_name[i]);
	}
}

void load_device_name()
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		device_name[i] = (char)EEPROM.read(DEVICE_NAME_ADDRESS + i);
	}
}

void listen_for_device_name()
{
	Serial.print(F("Current device name is: "));
	print_device_name();
	Serial.println();

	Serial.print(F("Enter new device name (max "));
	Serial.print(DEVICE_NAME_MAX_LENGTH);
	Serial.println(F(" characters):"));
	
	serial_recieve_index = 0;
	clearSerial();

	bool keepReading = true;
	while (keepReading)
	{
		while (Serial.available())
		{
			if (Serial.peek() == 13 || Serial.peek() == 10)
			{
				// new line. End entry
				keepReading = false;
				break;
			}
			serial_recieve_data[serial_recieve_index++] = Serial.read();
			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
			{
				// max length. End entry
				keepReading = false;
				break;
			}
		}
	}
	clearSerial();

	if (serial_recieve_index == 0)
	{
		// Empty, do not save.
		Serial.println(F("No input detected. No changes made."));
	}
	else
	{
		// Echo & save new name
		Serial.print(F("Device name set to: "));
		byte i;
		for (i = 0; i < serial_recieve_index; i++)
		{
			device_name[i] = (char)serial_recieve_data[i];
			Serial.print(device_name[i]);
		}
		// Fill rest of device name array with null chars
		for (; i <= DEVICE_NAME_MAX_LENGTH; i++)
		{
			device_name[i] = '\0';
		}
		Serial.println();
		save_device_name();
	}
}
/**
 * End: Device name
 */

/**
 * Start: Server IP
 */
void print_auth_server_ip()
{
	for (byte i = 0; i < 4; i++)
	{
		Serial.print(authServerIpBytes[i]);
		if (i != 3)
		{
			Serial.print('.');
		}
	}
}

void save_auth_server_ip()
{
	Serial.println("Saving Auth server IP Address...");
	for (byte i = 0; i < 4; i++)
	{
		EEPROM.write(AUTH_SERVER_IP_ADDRESS + i, authServerIpBytes[i]);
	}
}

void load_auth_server_ip()
{
	Serial.println("Loading authentcation server IP Address from EEPROM (if available)...");
	for (byte i = 0; i < 4; i++)
	{
		authServerIpBytes[i] = EEPROM.read(AUTH_SERVER_IP_ADDRESS + i);
		Serial.print("Auth server IP - ");
		Serial.println(authServerIpBytes[i], DEC);
	}
	//server = new IPAddress(authServerIpBytes[0], authServerIpBytes[1], authServerIpBytes[2], authServerIpBytes[3]);
}

void listen_for_auth_server_ip()
{
	Serial.print(F("Current auth server IP is: "));
	print_auth_server_ip();
	Serial.println();

	Serial.println(F("Enter new auth IP:"));
	serial_recieve_index = 0;
	clearSerial();

	serialListenForInput(15, -1, true, true, true);
	byte newIp[] = {0, 0, 0, 0};
	bool success = parseIpFromSerialRecieveData(authServerIpBytes);
	if (success)
	{
		Serial.println(F("Saving changes..."));
		save_auth_server_ip();
		Serial.print(F("Authentication server IP: "));
		print_auth_server_ip();
		Serial.println();
	}
	else
	{
		Serial.println(F("Invalid IP address supplied for authentication server. Changes not saved"));
	}
}

/**
 * End: Server IP
 */



/**
 * Start: E-Stop
 */
void eStopInit ()
{
	// Configure E-Stop pin
	pinMode(ESTOP_PIN, INPUT);
	digitalWrite(ESTOP_PIN, HIGH);
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}

void eStopBroadcast ()
{
	// TODO: Implement
}

void eStopInterruptHandler ()
{
	actuator_off();
	lockdown = true;
}

void eStopLockdown ()
{
	detachInterrupt(ESTOP_INTERRUPT_PIN); // disable estop interrupt so repeated estop presses won't cause weird/mistimed LED blinks.
	Serial.println(F("LOCKED DOWN!!!"));
	eStopBroadcast();
	ledOn(RED);
	blinkLed(GREEN);
	while (true)
	{
		// Lockdown loop
		manageLeds();
	}
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}
/**
 * End: E-Stop
 */