#ifndef DOORACTUATOR_H
#define DOORACTUATOR_H
/**
 * Actuators should have these methods:
 * + void actuator_init() - Run once at the start to set any required variables/pins/etc.
 * + void actuator_on() - Run whenever a valid authentication request has occured
 * + void actuator_off() - Run when it's time to shutodown the actuator (due to timeout/etc/etc).
 *
 * The door actuator will hold a pin high for 2 seconds to unlock a door strike. Off does nothing.
 */
#include "IActuator.h"
#include "Arduino.h"
#include "configuration.h"
#include "led.h"

void actuator_init ();
void actuator_on();
void actuator_off();

#endif // DOORACTUATOR_H