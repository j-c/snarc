#ifndef UTILS_H
#define UTILS_H

#include "configuration.h"
#include <EEPROM.h>
#include <stdio.h>

void clearSerial ();
void serialListenForInput (int readLength, unsigned long timeout, bool endOnNewLine, bool clearBufferBefore, bool clearBufferAfter, char *serial_recieve_data);
bool parseIpFromSerialRecieveData(byte *result);
uint16_t calcCrc16(unsigned char *plainTextArray, int plaintextLength);
uint8_t packetHelper(unsigned char* packet, uint8_t max_size, const char *fmt, ... );
bool get_server_token(unsigned char* token);

#ifdef ETHERNET
void initEthernet ();
void print_mac_address ();
void generate_random_mac_address (uint8_t *mac);
void save_mac_address (uint8_t *mac);
void get_mac_address (uint8_t *mac);
void print_ip_address();
void save_ip_address();
void get_ip_address (uint8_t *clientIpBytes);
void listen_for_ip_address();
#endif // ETHERNET

void print_device_name();
void save_device_name(char* device_name);
void get_device_name(char* device_name);
void listen_for_device_name();
void print_auth_server_ip();
void save_auth_server_ip();
void get_auth_server_ip(uint8_t* authServerIpBytes);
void listen_for_auth_server_ip();

#endif // UTILS_H
