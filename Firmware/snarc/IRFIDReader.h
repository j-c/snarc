#ifndef IRIFDREADER_H
#define IRIFDREADER_H

#include <HardwareSerial.h>

class IRFIDReader
{
public:
	virtual bool available() = 0;
	virtual unsigned long read() = 0;
	void setHardwareSerial (HardwareSerial *serialInstance)
	{
		hardwareSerial = serialInstance;
	}
private:
	HardwareSerial *hardwareSerial;
};

#endif