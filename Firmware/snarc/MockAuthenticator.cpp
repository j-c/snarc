#include "MockAuthenticator.h"

MockAuthenticator::MockAuthenticator()
{
	// Nothing
}

bool MockAuthenticator::authenticate(unsigned long id, char * deviceName)
{
	return id == 0xFFFFFFFF;
}