#ifndef HSBNE_AUTHENTICATOR_H_
#define HSBNE_AUTHENTICATOR_H_

/**
 * Authenticatoion providers should have these methods:
 * + void authenticator_init() - Run once at the start to set any required variables/pins/etc.
 * + bool authenticator_authenticate(unsigned long id, char * deviceName) - Run whenever we want to authenticate an ID.
 *
 * This is a mock authenticator for testing purposes. It will accept 0xFFFFFFFF and reject everything else.
 */

#include <SPI.h>
#include <Ethernet.h>
#include "utils.h"

#define AUTH_SERVER_PORT 8001

extern EthernetClient client;
extern byte authServerIpBytes[];

boolean authenticator_connected = false;

void authenticator_init()
{
	Serial.print(F("Opening connection to "));
	for (int i = 0; i < 4; i++)
	{
		Serial.print(authServerIpBytes[i]);
		Serial.print('.');
	}
	Serial.print(".. port ");
	Serial.print(AUTH_SERVER_PORT, DEC);
	Serial.println("... ");
	delay(5000);
	IPAddress serverIp(authServerIpBytes[0], authServerIpBytes[1], authServerIpBytes[2], authServerIpBytes[3]);
	if (client.connect(serverIp, AUTH_SERVER_PORT))
	{
		Serial.println(F("Connected!"));
		authenticator_connected = true;
	}
	else
	{
		Serial.println(F("Failed!"));
	}
}

bool authenticator_authenticate(unsigned long id, char * deviceName)
{
	unsigned char token[255];
	uint8_t input[255];
	int i = 0;
	input[0] = (uint8_t) 'T';
	input[1] = 0;
	client.write(input, 2);
	//client.write('T');
	//client.write((byte)0);
	delay(1000);
	while (client.available())
	{
		unsigned char c = client.read();
		Serial.print(c, HEX);
		Serial.print(' ');
		token[i++] = c;
	}
	
	Serial.println();

	uint16_t crc = calcCrc16(token, 3);
	// Here be dragons!
	if (crc == *((uint16_t*)(token + 3)))
	{
		Serial.println("CRC good");
	}
	else
	{
		Serial.println("CRC ARAFSADSAGDAGSETHDRGFTRHJ");
/*		Serial.print(crc & 0xFF, HEX);
		Serial.print(' ');
		Serial.println(crc >> 8, HEX);
		Serial.print(token[3], HEX);
		Serial.print(' ');
		Serial.println(token[4], HEX);
		Serial.print("crc: ");
		Serial.println(crc, HEX);
		Serial.print("tok: ");
		Serial.println(*((uint16_t*)(token + 3)), HEX);
		*/
	}
	return id == 0xFFFFFFFF;
}

#endif