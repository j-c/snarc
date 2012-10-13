#ifndef MOCKRFIDREADER_H_
#define MOCKRFIDREADER_H_

//#include "configuration.h"
#include "IRFIDReader.h"
#include <SoftwareSerial.h>
#include "Arduino.h"

class MockRFIDReader : public IRFIDReader
{
public:
	MockRFIDReader ();
	bool available();
	unsigned long read();
private:
	unsigned long nextAvailable;
	bool nextIdZero; // If next request for an ID should return a 0 or 0xFFFFFFFF;
};

#endif