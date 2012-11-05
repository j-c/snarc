#ifndef LED_H
#define LED_H

#include "configuration.h"

#define LED_BLINK_INTERVAL 750; // interval at which to blink (milliseconds)
#define NUM_LEDS 2

enum LedState
{
    ON,
    BLINK,
    FLASH,
    OFF
};

enum LedColour
{
    RED = 0,
    GREEN = 1
};

void flashLed (LedColour led, unsigned long flashLength);
void ledOn (LedColour led);
void ledOff (LedColour led);
void blinkLed (LedColour led);
void manageLeds ();

#endif // LED_H
