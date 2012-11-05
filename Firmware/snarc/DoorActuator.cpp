/**
 * Actuators should have these methods:
 * + void actuator_init() - Run once at the start to set any required variables/pins/etc.
 * + void actuator_on() - Run whenever a valid authentication request has occured
 * + void actuator_off() - Run when it's time to shutodown the actuator (due to timeout/etc/etc).
 *
 * The door actuator will hold a pin high for 2 seconds to unlock a door strike. Off does nothing.
 */
#include "DoorActuator.h"

unsigned long actuator_momentaryTime = 2000; // TODO: Make configurable

void actuator_init ()
{
	pinMode(ACTUATOR_PIN, OUTPUT);
}

void actuator_on()
{
	digitalWrite(ACTUATOR_PIN, HIGH);
	unsigned long delayEnd = millis() + actuator_momentaryTime;
	while (actuator_momentaryTime > millis())
	{
		manageLeds();
	}
	digitalWrite(ACTUATOR_PIN, LOW);
}


void actuator_off()
{
	// Nothing
}
