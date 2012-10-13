#ifndef DOORACTUATOR_H_
#define DOORACTUATOR_H_

#include "IActuator.h"
#include "Arduino.h"
#include "configuration.h"

class DoorActuator : public IActuator
{
public:
	DoorActuator();
	void on();
	void off();
private:
	int momentaryTime;
};

#endif