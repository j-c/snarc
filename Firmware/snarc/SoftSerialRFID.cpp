/**
 * Authenticatoion providers should have these methods:
 * + void reader_init() - Run once at the start to set any required variables/pins/etc.
 * + boolean reader_available() - Will return true if a full ID is available to.
 * + unsigned long reader_read() - Returns the latest ID. Will cause reader_available() to return false till another new ID is read.
 *
 * This is a mock reader is for testing purposes. It will alternate between returning 0x000000 and 0xFFFFFFFF every 5 seconds.
 */

#include "SoftSerialRFID.h"

int reader_tagStringIndex;
char reader_tagString[10];
SoftwareSerial reader_rfidSerial(15, 16);
unsigned int reader_tag;
bool reader_mTagAvailable;

void reader_init()
{
	reader_rfidSerial.begin(2400);
	reader_mTagAvailable = false;
	reader_tagStringIndex = 0;
	reader_tag = 0;
}

unsigned long reader_read()
{
	if (!reader_mTagAvailable)
	{
		reader_tag = 0;
		return reader_tag;
	}

	reader_tag = 0;
	byte i;
	for (i = 0; i < 10; i++)
	{
		char val = reader_tagString[i];
		if (val == 10 || val == 13 || val == 0)
		{
			break;
		}
		reader_tag = reader_tag * 10 + (val - 48);
	}
	if (i != 10)
	{
		// less than 10 bytes of data. Fail!
		reader_tag = 0;
	}
	else
	{
		// read successful wipe tag string
		for (i = 0; i < 10; i++)
		{
			reader_tagString[i] = 0;
		}
	}
	reader_mTagAvailable = false;
	return reader_tag;
}

void reader_readTag ()
{
	if (reader_mTagAvailable)
	{
		return;
	}

	reader_tagStringIndex = 0;
	if (reader_rfidSerial.available() >= 10)
	{
		while (reader_rfidSerial.available())
		{
			reader_tagString[reader_tagStringIndex++] = reader_rfidSerial.read();
		}
		reader_mTagAvailable = true;
	}
}

bool reader_available()
{
	reader_readTag();
	return reader_mTagAvailable;
}