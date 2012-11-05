// https://github.com/j-c/snarc

#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "snarc.h"

#ifdef ENABLE_ESTOP
	extern bool lockdown;
#endif

// Single RED FLASH with BLINKING GREEN programming mode. Will FLASH RED with each command.
// Single RED FLASH during operation means card ID does not have access.
// Single GREEN FLASH during operation means card ID has access.
// SOLID RED with BLINKING GREEN means E-Stop condition.

/* USAGE: be sure to revise this script for each peice of hardware:
 * change the IP address that it should be assigned to ( default is 192.168.7.7 ) 
 * make sure the "server" it will authenticate to is correct ( default is 192.168.7.77 ) 
 * make sure the URL that is being used for your "auth server" is correct.  ( default is /auth.php? ) 
 * make sure you have it wired-up correctly! Code currently assumes the RFID reader is on Software Serial on RX/TX pins  D15 and D16 ( also sometimes called A1 and A2 )
*/

#ifdef ETHERNET
#include <SPI.h> //needed by Ethernet.h
#include <Ethernet.h>

byte authServerIpBytes[] = {0, 0, 0, 0};
//IPAddress server(0, 0, 0, 0); // destinaton end-point is "auth server" internal IP.
EthernetClient client;

bool ethernetInit = false;

#endif

void setup()
{
	// Needed for random MAC address
	randomSeed(analogRead(A1));

	Serial.begin(SNARC_BAUD_RATE);

	Serial.println();

	programmingMode();

	// Display basic config details
	Serial.println();
	Serial.print("Name: ");
	print_device_name();
	Serial.println();

	#ifdef ETHERNET
	Serial.println("Starting ethernet...");
	initEthernet();
	#endif

	// Actuator init
	pinMode(ACTUATOR_PIN, OUTPUT);
	digitalWrite(ACTUATOR_PIN, LOW);

	#ifdef ENABLE_ESTOP
	eStopInit();
	#endif

	Serial.println("Started");

	// Init status LEDs
	pinMode(RED_LED_PIN, OUTPUT);
	digitalWrite(RED_LED_PIN, LOW);
	pinMode(GREEN_LED_PIN, OUTPUT);
	digitalWrite(GREEN_LED_PIN, LOW);

	delay(3000);
	authenticator_init();

	Serial.println("Entering operation mode! ");
}


void loop ()
{
	manageLeds();

	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		eStopLockdown();
	}
	#endif

	
	// Check to see if card is available
	if (reader_available())
	{
		#ifdef DEBUG
				Serial.print("Memory free: ");
				Serial.println(freeMemory());
		#endif
		unsigned long id = reader_read();
		if (authenticator_authenticate(id))
		{
			// Success
			flashLed(GREEN, 2000);
			manageLeds();
			Serial.println("ID has access");
			actuator_on();
		}
		else
		{
			// Fail
			flashLed(RED, 1000);
			actuator_off();
			Serial.println("ID does not have access");
			// TODO: Implement timeout and turn off if no valid card found
		}
		// TODO: wait/sleep for a while before attempting read again?
	}

}


/**
 * Start: E-Stop
 */
void eStopInit ()
{
	// Configure E-Stop pin
	pinMode(ESTOP_PIN, INPUT);
	digitalWrite(ESTOP_PIN, HIGH);
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}

void eStopBroadcast ()
{
	// TODO: Implement
}

void eStopInterruptHandler ()
{
	actuator_off();
	lockdown = true;
}

void eStopLockdown ()
{
	detachInterrupt(ESTOP_INTERRUPT_PIN); // disable estop interrupt so repeated estop presses won't cause weird/mistimed LED blinks.
	Serial.println("LOCKED DOWN!!!");
	eStopBroadcast();
	ledOn(RED);
	blinkLed(GREEN);
	while (true)
	{
		// Lockdown loop
		manageLeds();
	}
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}
/**
 * End: E-Stop
 */
