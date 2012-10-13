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
 * Setting maxLength to -1 will read for as long as there is space in serial_recieve_data[].
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
#endif