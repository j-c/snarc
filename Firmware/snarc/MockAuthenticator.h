#ifndef MOCK_AUTHENTICATOR_H_
#define MOCK_AUTHENTICATOR_H_

/**
 * Authenticatoion providers should have these methods:
 * + void authenticator_init() - Run once at the start to set any required variables/pins/etc.
 * + bool authenticator_authenticate(unsigned long id, char * deviceName) - Run whenever we want to authenticate an ID.
 *
 * This is a mock authenticator for testing purposes. It will accept 0xFFFFFFFF and reject everything else.
 */

void authenticator_init()
{
	// Nothing
}

bool authenticator_authenticate(unsigned long id, char * deviceName)
{
	return id == 0xFFFFFFFF;
}

#endif