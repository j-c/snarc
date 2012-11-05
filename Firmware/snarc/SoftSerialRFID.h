#ifndef SOFTSERIALRFID_H
#define SOFTSERIALRFID_H

/**
 * Authenticatoion providers should have these methods:
 * + void reader_init() - Run once at the start to set any required variables/pins/etc.
 * + boolean reader_available() - Will return true if a full ID is available to.
 * + unsigned long reader_read() - Returns the latest ID. Will cause reader_available() to return false till another new ID is read.
 *
 * This is a mock reader is for testing purposes. It will alternate between returning 0x000000 and 0xFFFFFFFF every 5 seconds.
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

void reader_init();
unsigned long reader_read();
void reader_readTag ();
bool reader_available();

#endif // SOFTSERIALRFID_H