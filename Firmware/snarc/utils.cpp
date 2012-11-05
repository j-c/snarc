#include "utils.h"

extern EthernetClient client;

/**
 * Clears anything in the serial recieve buffer
 */
void clearSerial ()
{
	while (Serial.available())
	{
		Serial.read();
	}
}

/**
 * Blocking operation. Listens for input from the serial port.
 * Exits when a newline is received, the maximum length is reached, or timeout is reached.
 * Setting readLength to -1 will read for as long as there is space in serial_recieve_data[].
 * Setting timeout to -1 will allow this method to block for as long as it's needed to reach the max length or a newline.
 * Setting endOnNewLine to true will cause the method to stop reading on a new line
 * Setting clearBufferBefore will clear the serial buffer before waiting for input.
 * Setting clearBufferAFter to true will clear the rest of the data in the Serial read buffer after competion.
 * When more data is sent than there is space, it will clear the serial buffer till the next newline (or till empty)
 */
void serialListenForInput (int readLength, unsigned long timeout, bool endOnNewLine, bool clearBufferBefore, bool clearBufferAfter, char* serial_recieve_data)
{
	bool reading = true;
	unsigned long timeoutMillis = timeout + millis();
	int maxLength = readLength > -1 ? readLength : SERIAL_RECIEVE_BUFFER_LENGTH;
	int serial_recieve_index = 0;
	
	if (clearBufferBefore)
	{
		clearSerial();
	}
	
	while (millis() <= timeoutMillis && serial_recieve_index <= maxLength)
	{
		while (Serial.available())
		{
			serial_recieve_data[serial_recieve_index++] = Serial.read();
			
			if (serial_recieve_index >= maxLength-1)
			{
				reading = false;
				break;
			}
			else if (serial_recieve_data[serial_recieve_index-1] == '\n' || serial_recieve_data[serial_recieve_index-1] == '\r')
			{
				reading = false;
				break;
			}
		}
	}
	if (clearBufferAfter)
	{
		clearSerial();
	}
	serial_recieve_data[serial_recieve_index++] = 0; // Add a null at the end of the string
}

/*
unsigned long stringToULong (char * uLongString)
{
	int i = 0;
	unsigned long output = 0;
	while (uLongString[i] != '\0')
	{
		output = output * 10 + (uLongString[i++] - 48);
	}
	return output;
}
*/

//bool parseIpFromSerialRecieveData(byte *result)
//{
//	boolean validInput = true;
//	int octet = 0;
//	byte newIp[] = {0, 0, 0, 0};
//	byte newIpIndex = 0;
//	for (byte i = 0; i <= serial_recieve_index; i++) // Iterating 1 extra time because we won't always have an new line character. So it's used to put the last octet into the array.
//	{
//		if (i == serial_recieve_index)
//		{
//			newIp[newIpIndex++] = octet;
//			break;
//		}
//
//		byte b = serial_recieve_data[i];
//		if (b >= 48 && b <= 57) // 0-9
//		{
//			octet = octet * 10 + (b - 48);
//			if (octet > 255)
//			{
//				validInput = false;
//				break;
//			}
//		}
//		else if ((char)b == '.') // TODO: replace with ASCII code for '.' (I think having ascii is better.. becomes more readable)
//		{
//			newIp[newIpIndex++] = octet;
//			octet = 0;
//		}
//		else if (b == 10 || b == 13)
//		{
//			newIp[newIpIndex++] = octet;
//			break;
//		}
//		else // invalid char
//		{
//			validInput = false;
//			break;
//		}
//	}
//	
//	if (validInput)
//	{
//		if (newIp[0] == 0 || newIp[3] == 0)
//		{
//			validInput = false;
//		}
//		else if (newIp[0] == 0 && newIp[1] == 0 && newIp[2] == 0 && newIp[3] == 0)
//		{
//			validInput = false;
//		}
//	}
//
//	if (validInput)
//	{
//		result[0] = newIp[0];
//		result[1] = newIp[1];
//		result[2] = newIp[2];
//		result[3] = newIp[3];
//	}
//	return validInput;
//}

uint16_t calcCrc16(unsigned char *plainTextArray, int plaintextLength)
{
	uint16_t crc = 0;
	for (int i = 0; i < plaintextLength; i++)
	{
		crc = crc ^ plainTextArray[i] << 8;
		for (int j = 0; j < 8; j++)
		{
			
			if (crc & 0x8000)
			{
				crc = crc << 1 ^ 0x1021;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}
	return crc;
}

uint8_t packetHelper(unsigned char* packet, uint8_t max_size, const char *fmt, ... )
{
    uint8_t n;
    va_list ap;
    // write the message into a string, place at offset 1 (first byte is the application type)
    va_start( ap, fmt );
    n = vsnprintf( (char*) packet, max_size, fmt, ap );
    va_end( ap );
    
    return n;
}

bool get_server_token(unsigned char* token)
{
	uint8_t input[2];
	int i = 0;
	input[0] = (uint8_t) 'T';
	input[1] = 0;
	client.write(input, 2);
	delay(1000);
	while (client.available())
	{
		token[i++] = client.read();
	}
	
	uint16_t crc = calcCrc16(token, 3);
	
	// If our calculated crc is the same as the one in the packet everything is ok.
	if (crc == *((uint16_t*)(token + 3)))
	{
		Serial.println("Token CRC good");
		return true;
	}
	return false;
}

#ifdef ETHERNET
void initEthernet ()
{
byte mac[6];
IPAddress ip(0, 0, 0, 0);
uint8_t clientIpBytes[4];
	bool ethernetInit = true; // This is actually quite bad, but currently ethernet init status is set to true as a quick fix. Most failure modes seem to lockup the Arduino anyway.
	get_ip_address(clientIpBytes);
	get_auth_server_ip(clientIpBytes);
	get_mac_address(mac);
	ip = IPAddress(clientIpBytes[0], clientIpBytes[1], clientIpBytes[2], clientIpBytes[3]);

	bool useDhcp = (clientIpBytes[0] == 0 && clientIpBytes[1] == 0 && clientIpBytes[2] == 0 && clientIpBytes[3] == 0) || (clientIpBytes[0] == 0xff && clientIpBytes[1] == 0xff && clientIpBytes[2] == 0xff && clientIpBytes[3] == 0xff);
	if (useDhcp)
	{
		Serial.println("Attempting to aquire IP via DHCP...");
		if (Ethernet.begin(mac) == 0)
		{
			Serial.println("Failed to configure Ethernet using DHCP");
			ethernetInit = false;
			// TODO: Do something aout this error
		}
	}
	else
	{
		Ethernet.begin(mac, ip);
	}

	Serial.print("                      IP: ");
	print_ip_address();
	Serial.println();

	Serial.print("                     MAC: ");
	print_mac_address();
	Serial.println();

	Serial.print("Authentication server IP: ");
	print_auth_server_ip();
	Serial.println();
}


/**
 * Start: Mac Address
 */
void print_mac_address ()
{
        uint8_t mac[6];
        char macString[18];
        get_mac_address (mac);
        printf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        Serial.println(macString);
}

void save_mac_address (uint8_t *mac)
{
	for (byte i = 0; i < 6; i++)
	{
		EEPROM.write(MAC_START_ADDRESS + i, mac[i]);
	}
}

void generate_random_mac_address (uint8_t *mac)
{
        for (uint8_t i = 0; i < 6; i++)
        {
          mac[i] = random(0, 0xFF);
        }
  	save_mac_address(mac);
}

void get_mac_address (uint8_t *mac)
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
		Serial.print("Generating random MAC address: ");
		generate_random_mac_address(mac);
	}
}

/**
 * Start: IP Address
 */
void print_ip_address()
{
        uint32_t clientIpBytes;
	Serial.print(Ethernet.localIP());
        get_ip_address ((uint8_t*) &clientIpBytes);
	/*
	Serial.print(clientIpBytes[0], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[1], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[2], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[3], DEC);
	*/
	if (clientIpBytes == 0)
	{
		Serial.print(" (via DHCP)");
	}

}

void save_ip_address(uint8_t *clientIpBytes)
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

void get_ip_address (uint8_t *clientIpBytes)
{
	Serial.println("Loading IP Address from EEPROM (if available)...");
	for (byte i = 0; i < 4; i++)
	{
		clientIpBytes[i] = EEPROM.read(IP_ADDRESS + i);
	}
}

//void listen_for_ip_address()
//{
//	Serial.print("Current ip address: ");
//	print_ip_address();
//
//	Serial.println("Enter new ip address (press enter to use DHCP):");
//	
//	int serial_recieve_index = 0;
//	clearSerial();
//
//	bool keepReading = true;
//	while (keepReading)
//	{
//		while (Serial.available())
//		{
//			if (Serial.peek() == 13 || Serial.peek() == 10)
//			{
//				// new line. End entry
//				keepReading = false;
//				break;
//			}
//			serial_recieve_data[serial_recieve_index++] = Serial.read();
//			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
//			{
//				// max length. End entry
//				keepReading = false;
//				break;
//			}
//		}
//	}
//	clearSerial();
//
//	if (serial_recieve_index == 0)
//	{
//		// Empty, use DHCP
//		uint32_t ipaddr = 0;
//		save_ip_address((uint8_t*) &ipaddr);
//		Serial.print("Using DHCP.");
//	}
//	else
//	{
//		boolean validInput = true;
//		int octet = 0;
//		byte newIp[] = {0, 0, 0, 0};
//		byte newIpIndex = 0;
//		for (byte i = 0; i < serial_recieve_index; i++)
//		{
//			if (serial_recieve_data[i] >= 48 && serial_recieve_data[i] <= 57) // 0-9
//			{
//				octet = octet * 10 + (serial_recieve_data[i] - 48);
//				if (octet > 255)
//				{
//					validInput = false;
//					break;
//				}
//			}
//			else if ((char)serial_recieve_data[i] == '.') // TODO: replace with ASCII code for '.'
//			{
//				newIp[newIpIndex++] = octet;
//				octet = 0;
//			}
//			else if (serial_recieve_data[i] == 10 || serial_recieve_data[i] == 13) // newline
//			{
//				if (octet != 0 || newIpIndex != 0) // no input parsed yet
//				{
//					validInput = false;
//				}
//				else
//				{
//					newIp[newIpIndex++] = octet;
//				}
//				break;
//			}
//			else // invalid char
//			{
//				validInput = false;
//				break;
//			}
//		}
//
//		if (validInput)
//		{
//			save_ip_address(newIp);
//			Serial.println(" Please reset SNARC for changes to take effect.");
//		}
//		else
//		{
//			Serial.println("Invalid input. IP address not changed.");
//		}
//	}
//}
/**
 * End: IP address
 */
#endif
 
 /**
 * Start: Server IP
 */
void print_auth_server_ip()
{
        uint8_t authServerIpBytes[4];
        get_auth_server_ip(authServerIpBytes);
	for (byte i = 0; i < 4; i++)
	{
		Serial.print(authServerIpBytes[i]);
		if (i != 3)
		{
			Serial.print('.');
		}
	}
}

void save_auth_server_ip(char* authServerIpBytes)
{
	Serial.println("Saving Auth server IP Address...");
	for (byte i = 0; i < 4; i++)
	{
		EEPROM.write(AUTH_SERVER_IP_ADDRESS + i, authServerIpBytes[i]);
	}
}

void get_auth_server_ip(uint8_t* authServerIpBytes)
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

//void listen_for_auth_server_ip(char* authServerIpBytes)
//{
//	Serial.print(F("Current auth server IP is: "));
//	print_auth_server_ip();
//	Serial.println();
//
//	Serial.println(F("Enter new auth IP:"));
//	int serial_recieve_index = 0;
//	clearSerial();
//
//	serialListenForInput(15, -1, true, true, true);
//	byte newIp[] = {0, 0, 0, 0};
//	if (parseIpFromSerialRecieveData(authServerIpBytes))
//	{
//		Serial.println(F("Saving changes..."));
//		save_auth_server_ip();
//		Serial.print(F("Authentication server IP: "));
//		print_auth_server_ip();
//		Serial.println();
//	}
//	else
//	{
//		Serial.println(F("Invalid IP address supplied for authentication server. Changes not saved"));
//	}
//}

/**
 * End: Server IP
 */
 
 
 
 /**
 * Start: Device name
 */
void save_device_name(char* device_name)
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		EEPROM.write(DEVICE_NAME_ADDRESS + i, (byte)device_name[i]);
	}
}

void get_device_name(char* device_name)
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		device_name[i] = (char)EEPROM.read(DEVICE_NAME_ADDRESS + i);
	}
}

void print_device_name()
{
   char device[DEVICE_NAME_MAX_LENGTH];
   get_device_name(device);
   Serial.println(device);
}
//void listen_for_device_name()
//{
//	Serial.print(F("Current device name is: "));
//	print_device_name();
//	Serial.println();
//
//	Serial.print(F("Enter new device name (max "));
//	Serial.print(DEVICE_NAME_MAX_LENGTH);
//	Serial.println(F(" characters):"));
//	
//	serial_recieve_index = 0;
//	clearSerial();
//
//	bool keepReading = true;
//	while (keepReading)
//	{
//		while (Serial.available())
//		{
//			if (Serial.peek() == 13 || Serial.peek() == 10)
//			{
//				// new line. End entry
//				keepReading = false;
//				break;
//			}
//			serial_recieve_data[serial_recieve_index++] = Serial.read();
//			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
//			{
//				// max length. End entry
//				keepReading = false;
//				break;
//			}
//		}
//	}
//	clearSerial();
//
//	if (serial_recieve_index == 0)
//	{
//		// Empty, do not save.
//		Serial.println(F("No input detected. No changes made."));
//	}
//	else
//	{
//		// Echo & save new name
//		Serial.print(F("Device name set to: "));
//		byte i;
//		for (i = 0; i < serial_recieve_index; i++)
//		{
//			device_name[i] = (char)serial_recieve_data[i];
//			Serial.print(device_name[i]);
//		}
//		// Fill rest of device name array with null chars
//		for (; i <= DEVICE_NAME_MAX_LENGTH; i++)
//		{
//			device_name[i] = '\0';
//		}
//		Serial.println();
//		save_device_name();
//	}
//}
/**
 * End: Device name
 */

