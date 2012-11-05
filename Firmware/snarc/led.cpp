#include "led.h"

unsigned long ledNextBlink = 0;

LedState ledStates[] = { OFF, OFF };
int ledPins[] = { RED_LED_PIN, GREEN_LED_PIN };
int ledPinOffState[] = { LOW, LOW };
unsigned long ledInterval[] = { 0, 0 };


void flashLed (LedColour led, unsigned long flashLength)
{
	ledStates[led] = FLASH;
	ledInterval[led] = millis() + flashLength;
}


void ledOn (LedColour led)
{
	ledStates[led] = ON;
}


void ledOff (LedColour led)
{
	ledStates[led] = OFF;
}


void blinkLed (LedColour led)
{
	ledStates[led] = BLINK;
}


void manageLeds ()
{
	unsigned long now = millis();
	for (int i = 0; i < NUM_LEDS; i++)
	{
		switch (ledStates[i])
		{
		case OFF:
			digitalWrite(ledPins[i], LOW);
			break;
		case ON:
			digitalWrite(ledPins[i], HIGH);
			break;
		case BLINK:
			if (ledNextBlink < now)
			{
				digitalWrite(ledPins[i], digitalRead(ledPins[i]) ^ 1); // Invert pin
			}
			break;
		case FLASH:
			if (ledInterval[i] <= now)
			{
				ledStates[i] = OFF;
				digitalWrite(ledPins[i], LOW);
				ledInterval[i] = 0;
			}
			else
			{
				digitalWrite(ledPins[i], HIGH);
			}
			break;
		default:
			break;
		}
	}
	if (ledNextBlink < now)
	{
		ledNextBlink = now + LED_BLINK_INTERVAL;
	}
}