// https://github.com/j-c/snarc

#ifndef	SNARC_H
#define SNARC_H

#include "configuration.h"

#ifdef ETHERNET
	#include <SPI.h> //needed by Ethernet.h
	#include <Ethernet.h>
#endif

#include "MemoryFree.h"
#include "configuration.h"
#include <EEPROM.h>

#include <SoftwareSerial.h>

#include "utils.h"
#include "led.h"
#include "ProgrammingMode.h"

#include "IRFIDReader.h"
#include "IAuthenticator.h"
#include "IActuator.h"

void eStopInit ();
void eStopBroadcast ();
void eStopInterruptHandler ();
void eStopLockdown ();

#endif // SNARC_H
