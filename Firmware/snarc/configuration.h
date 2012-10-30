#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// un-comment if debugging
#define DEBUG
#define SALT "sderf"
#define SNARC_BAUD_RATE 115200

#define ACTUATOR_PIN 14 // mosfet pin

// E-Stop functionality
#define ENABLE_ESTOP
#ifdef ENABLE_ESTOP
	//#define ESTOP_PIN 3 // SNARC board v1
	#define ESTOP_PIN 2
	//#define ESTOP_INTERRUPT_PIN 1 // SNARC board v1
	#define ESTOP_INTERRUPT_PIN 0
#endif

// Comment out to disable ethernet
#define ETHERNET

// Hardware config
//LED related variables:
#define RED_LED_PIN 5
#define GREEN_LED_PIN 6

// Internal software config
#define SERIAL_RECIEVE_BUFFER_LENGTH 32 // Length of serial read buffer
#define DEVICE_NAME_MAX_LENGTH 12 // Maximum length of the device name

// EEPROM address offsets for saving data
#define MAC_START_ADDRESS 0 // Where to save the mac address in the eeprom (needs 6 bytes)
#define IP_ADDRESS 6 // Where to save SNARC IP address (needs 4 bytes)
#define AUTH_SERVER_IP_ADDRESS 10 // Where to save Authentication server IP address (needs 4 bytes)
#define DEVICE_NAME_ADDRESS 14 // Where to save the device name
#define EEPROM_CARDS_START_ADDRESS 28 // Which address to start saving cards to. Should be the last address



// 0 = mock reader, 1 = soft serial
#define RFID_READER_TYPE 0

#if RFID_READER_TYPE == 0
	#include "MockRFIDReader.h"
#endif

#if RFID_READER_TYPE == 1
	#include "SoftSerialRFID.h"
#endif

// 0 = mock authenticator, 1 = HSBNE authenticator
#define AUTHENTICATOR_PROVIDER_TYPE 1

#if AUTHENTICATOR_PROVIDER_TYPE == 0
	#include "MockAuthenticator.h"
#endif

#if AUTHENTICATOR_PROVIDER_TYPE == 1
	#include "HsbneAuthenticator.h"
#endif

// 0 = non existant, 1 = DoorActuator
#define ACTUATOR_TYPE 1

#if ACTUATOR_TYPE == 1
	#include "DoorActuator.h"
#endif


#endif
