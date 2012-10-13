#ifndef IACTUATOR_H_
#define IACTUATOR_H_

class IActuator
{
public:
	virtual void on() = 0;
	virtual void off() = 0;
};

#endif

