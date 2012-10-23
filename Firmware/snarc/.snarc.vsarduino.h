#define __AVR_ATmega328P__
#define __cplusplus
#define __builtin_va_list int
#define __attribute__(x)
#define __inline__
#define __asm__(x)
#define ARDUINO 100
extern "C" void __cxa_pure_virtual() {}
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\EEPROM\EEPROM.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\SPI\SPI.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Dhcp.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Dns.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Ethernet.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetClient.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetServer.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetUdp.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\util.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\utility\socket.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\utility\w5100.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\SoftwareSerial\SoftwareSerial.h"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\EEPROM\EEPROM.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\SPI\SPI.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Dhcp.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Dns.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\Ethernet.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetClient.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetServer.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\EthernetUdp.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\utility\socket.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\Ethernet\utility\w5100.cpp"
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\libraries\SoftwareSerial\SoftwareSerial.cpp"
void setup();
void loop ();
void initEthernet ();
void print_mac_address ();
void save_mac_address ();
void generate_random_mac_address ();
void get_mac_address ();
void set_mac_address (byte octet0, byte octet1, byte octet2, byte octet3, byte octet4, byte octet5);
void listen_for_new_mac_address ();
void print_ip_address();
void save_ip_address();
void load_ip_address ();
void listen_for_ip_address();
void print_device_name();
void save_device_name();
void load_device_name();
void listen_for_device_name();
void print_auth_server_ip();
void save_auth_server_ip();
void load_auth_server_ip();
void listen_for_auth_server_ip();
void eStopInit ();
void eStopBroadcast ();
void eStopInterruptHandler ();
void eStopLockdown ();

#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Users\Joel\AppData\Local\Arduino\arduino-1.0\hardware\arduino\cores\arduino\Arduino.h"
#include "C:\Users\Joel\My Dropbox\Projects\Arduino\SNARC\Firmware\snarc\snarc.ino" 
