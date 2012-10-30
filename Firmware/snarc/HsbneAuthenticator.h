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

bool authenticator_authenticate(uint32_t card_id, char* deviceName)
{
	unsigned char token[5];
	unsigned char input[strlen(SALT)+2+8+strlen(deviceName)]; // Salt + Token + Msg (8+deviceName)
	unsigned char responce[255];
        va_list ap;
	uint8_t i = 0;
	
	while(!get_server_token(token))
	{
            packetHelper(input, strlen(SALT)+2+8+strlen(deviceName), "%c%c@%c%c%c", SALT, *((uint16_t*)(token+1)), strlen(deviceName), deviceName, card_id);
            *((uint16_t*)(input+10+strlen(deviceName))) = calcCrc16(input, input[4]+10);
            
            client.write(input+strlen(SALT)+2, 8+strlen(deviceName));
            delay(1000);
            while (client.available())
            {
                responce[i++] = client.read();
            }
            // Need to check the responce..
	}
	
	return card_id == 0xFFFF;
}

#endif
