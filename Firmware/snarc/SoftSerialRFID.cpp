#include "SoftSerialRFID.h"


SoftSerialRFID::SoftSerialRFID()
{
	SoftwareSerial softwareSerialInstance(15, 16);
	softwareSerialInstance.begin(2400);
	rfidSerial = &softwareSerialInstance;
	mTagAvailable = false;
	tagStringIndex = 0;
	tag = 0;
}

unsigned long SoftSerialRFID::read()
{
	if (!mTagAvailable)
	{
		tag = 0;
		return tag;
	}

	tag = 0;
	byte i;
	for (i = 0; i < 10; i++)
	{
		char val = tagString[i];
		if (val == 10 || val == 13 || val == 0)
		{
			break;
		}
		tag = tag * 10 + (val - 48);
	}
	if (i != 10)
	{
		// less than 10 bytes of data. Fail!
		tag = 0;
	}
	else
	{
		// read successful wipe tag string
		for (i = 0; i < 10; i++)
		{
			tagString[i] = 0;
		}
	}
	mTagAvailable = false;
	return tag;
}

void SoftSerialRFID::readTag ()
{
	if (mTagAvailable)
	{
		return;
	}

	tagStringIndex = 0;
	if (rfidSerial->available() >= 10)
	{
		while (rfidSerial->available())
		{
			tagString[tagStringIndex++] = rfidSerial->read();
		}
		mTagAvailable = true;
	}
}

bool SoftSerialRFID::available()
{
	readTag();
	return mTagAvailable;
}