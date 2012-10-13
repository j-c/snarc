#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// un-comment if debugging
#define DEBUG

#define SNARC_BAUD_RATE 115200

// 0 = mock reader, 1 = soft serial
#define RFID_READER_TYPE 0

#if RFID_READER_TYPE == 0
	#define RFID_READER_CLASS MockRFIDReader
	#include "MockRFIDReader.h"
#endif

#if RFID_READER_TYPE == 1
	#define RFID_READER_CLASS SoftSerialRFID
	#include "SoftSerialRFID.h"
#endif

// 0 = mock authenticator
#define AUTHENTICATOR_PROVIDER_TYPE 0

#if AUTHENTICATOR_PROVIDER_TYPE == 0
	#define AUTHENTICATOR_CLASS MockAuthenticator
	#include "MockAuthenticator.h"
#endif

// 0 = non existant, 1 = DoorActuator
#define ACTUATOR_TYPE 1

#if ACTUATOR_TYPE == 1
	#define ACTUATOR_CLASS DoorActuator
	#include "DoorActuator.h"
#endif

#define ACTUATOR_PIN 14 // mosfet pin

// E-Stop functionality
#define ENABLE_ESTOP
#ifdef ENABLE_ESTOP
	//#define ESTOP_PIN 3
	#define ESTOP_PIN 2
	//#define ESTOP_INTERRUPT_PIN 1
	#define ESTOP_INTERRUPT_PIN 0
#endif

// Comment out to disable ethernet
//#define ETHERNET

#define THISSNARC_OPEN_PIN 54  //54 on the mega, or 14 on the normal arduino
#define THISSNARC_RFID_ENABLE_PIN 2


// Hardware config
//LED related variables:
#define RED_LED_PIN 5
#define GREEN_LED_PIN 6

// Internal software config
#define SERIAL_RECIEVE_BUFFER_LENGTH 32 // Length of serial read buffer
#define DEVICE_NAME_MAX_LENGTH 12 // Maximum length of the device name

// EEPROM address offsets for saving data
#define MAC_START_ADDRESS 0 // Where to save the mac address in the eeprom (needs 6 bytes)
#define IP_ADDRESS 6 // Where to save IP address (needs 4 bytes)
#define DEVICE_NAME_ADDRESS 10 // Where to save the device name
#define EEPROM_CARDS_START_ADDRESS 24 // Which address to start saving cards to. Should be the last address

#endif