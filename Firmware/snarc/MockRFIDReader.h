#ifndef MOCKRFIDREADER_H_
#define MOCKRFIDREADER_H_

/**
 * Authenticatoion providers should have these methods:
 * + void reader_init() - Run once at the start to set any required variables/pins/etc.
 * + boolean reader_available() - Will return true if a full ID is available to.
 * + unsigned long reader_read() - Returns the latest ID. Will cause reader_available() to return false till another new ID is read.
 *
 * This is a mock reader is for testing purposes. It will alternate between returning 0x000000 and 0xFFFFFFFF every 5 seconds.
 */

#include <Arduino.h>

unsigned long reader_nextAvailable;
bool reader_nextIdZero; // If next request for an ID should return a 0 or 0xFFFFFFFF;

void reader_init()
{
	// Nothing
	reader_nextAvailable = millis();
	reader_nextIdZero = false;
}

bool reader_available()
{
	if (millis() >= reader_nextAvailable)
	{
		reader_nextAvailable = millis() + 5000; // Next card will be available in 5 seconds
		return true;
	}
	else
	{
		return false;
	}
}

unsigned long reader_read()
{
	reader_nextIdZero = !reader_nextIdZero;
	return reader_nextIdZero ? 4294967295 : 0;
}

#endif