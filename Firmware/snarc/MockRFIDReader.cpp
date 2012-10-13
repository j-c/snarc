#include "MockRFIDReader.h"

MockRFIDReader::MockRFIDReader()
{
	// Nothing
	nextAvailable = millis();
	nextIdZero = false;
}

bool MockRFIDReader::available()
{
	if (millis() >= nextAvailable)
	{
		nextAvailable = millis() + 5000; // Next card will be available in 5 seconds
		return true;
	}
	else
	{
		return false;
	}
}

unsigned long MockRFIDReader::read()
{
	nextIdZero = !nextIdZero;
	return nextIdZero ? 4294967295 : 0;
}