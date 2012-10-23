#ifndef UTILS_H_
#define UTILS_H_

#include "configuration.h"

// Temporary storage & output for serial read
char serial_recieve_data[SERIAL_RECIEVE_BUFFER_LENGTH];
int serial_recieve_index = 0;

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
void serialListenForInput (int readLength, long timeout, bool endOnNewLine, bool clearBufferBefore, bool clearBufferAfter)
{
	if (clearBufferBefore)
	{
		clearSerial();
	}
	serial_recieve_index = 0;
	bool reading = true;
	long timeoutMillis = timeout > -1 ? (timeout + millis()) : -1;
	int maxLength = readLength > -1 ? readLength : SERIAL_RECIEVE_BUFFER_LENGTH;
	while (reading)
	{
		while (Serial.available())
		{
			int c = Serial.read();
			if (c == 10 || c == 13)
			{
				// End
				reading = false;
				break;
			}
			serial_recieve_data[serial_recieve_index++] = (char)c;
			if (serial_recieve_index >= maxLength)
			{
				// Reached limit.
				reading = false;
				break;
			}
		}
		if (timeout > -1 && millis() >= timeout)
		{
			// Reached timeout
			reading = false;
			break;
		}
	}
	if (clearBufferAfter)
	{
		clearSerial();
	}
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

boolean parseIpFromSerialRecieveData(byte *result)
{
	boolean validInput = true;
	int octet = 0;
	byte newIp[] = {0, 0, 0, 0};
	byte newIpIndex = 0;
	for (byte i = 0; i <= serial_recieve_index; i++) // Iterating 1 extra time because we won't always have an new line character. So it's used to put the last octet into the array.
	{
		if (i == serial_recieve_index)
		{
			newIp[newIpIndex++] = octet;
			break;
		}

		byte b = serial_recieve_data[i];
		if (b >= 48 && b <= 57) // 0-9
		{
			octet = octet * 10 + (b - 48);
			if (octet > 255)
			{
				validInput = false;
				break;
			}
		}
		else if ((char)b == '.') // TODO: replace with ASCII code for '.'
		{
			newIp[newIpIndex++] = octet;
			octet = 0;
		}
		else if (b == 10 || b == 13)
		{
			newIp[newIpIndex++] = octet;
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
		if (newIp[0] == 0 || newIp[3] == 0)
		{
			validInput = false;
		}
		else if (newIp[0] == 0 && newIp[1] == 0 && newIp[2] == 0 && newIp[3] == 0)
		{
			validInput = false;
		}
	}

	if (validInput)
	{
		result[0] = newIp[0];
		result[1] = newIp[1];
		result[2] = newIp[2];
		result[3] = newIp[3];
	}
	return validInput;
}

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

#endif