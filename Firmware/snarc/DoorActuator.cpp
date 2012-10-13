#include "DoorActuator.h"

DoorActuator::DoorActuator(){
	momentaryTime = 2000; // TODO: Make configurable
	pinMode(ACTUATOR_PIN, OUTPUT);
};

void DoorActuator::on()
{
	digitalWrite(ACTUATOR_PIN, HIGH);
	unsigned long delayEnd = millis() + momentaryTime;
	while (momentaryTime > millis())
	{
		//manageLeds();
	}
	digitalWrite(ACTUATOR_PIN, LOW);
}

void DoorActuator::off()
{
	// Nothing
}