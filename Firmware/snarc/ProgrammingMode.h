#ifndef PROGRAMMING_MODE_H
#define PROGRAMMING_MODE_H

#include "snarc.h"
#include "configuration.h"
#include "utils.h"
#include <EEPROM.h>

#define EEPROM_MAX_ADDRESS 512

/**
 * Detects if serial data is entered within 3 seconds. If so enter programming mode.
 * Keeps looping in this method till the user exists programming mode.
 */
void programmingMode ();
void printProgrammingModeHelp ();
void wipeEeprom();

#ifdef ETHERNET
void listen_for_new_mac_address ();
#endif

#endif // PROGRAMMING_MODE_H

