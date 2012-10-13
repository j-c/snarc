#ifndef SOFTSERIALRFID_H
#define SOFTSERIALRFID_H

//#include "configuration.h"
#include "IRFIDReader.h"
#include <SoftwareSerial.h>
#include "Arduino.h"

class SoftSerialRFID : public IRFIDReader
{
public:
	SoftSerialRFID ();
	bool available();
	unsigned long read();
private:
	uint8_t tagStringIndex;
	char tagString[10];
	SoftwareSerial * rfidSerial;
	unsigned int tag;
	void readTag();
	bool mTagAvailable;
};

#endif