#ifndef IAUTHENTICATOR_H_
#define IAUTHENTICATOR_H_

class IAuthenticator
{
public:
	virtual bool authenticate (unsigned long id, char * deviceName) = 0;
};

#endif