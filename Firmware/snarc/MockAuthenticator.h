#ifndef MOCK_AUTHENTICATOR_H_
#define MOCK_AUTHENTICATOR_H_

#include "IAuthenticator.h"

class MockAuthenticator : public IAuthenticator
{
public:
	MockAuthenticator ();
	bool authenticate (unsigned long id, char * deviceName);
private:
	// nothing
};


#endif