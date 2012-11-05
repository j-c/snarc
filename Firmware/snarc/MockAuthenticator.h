#ifndef MOCK_AUTHENTICATOR_H
#define MOCK_AUTHENTICATOR_H

/**
 * Authenticatoion providers should have these methods:
 * + void authenticator_init() - Run once at the start to set any required variables/pins/etc.
 * + bool authenticator_authenticate(unsigned long id, char * deviceName) - Run whenever we want to authenticate an ID.
 *
 * This is a mock authenticator for testing purposes. It will accept 0xFFFFFFFF and reject everything else.
 */

void mock_authenticator_init();
bool mock_authenticator_authenticate(unsigned long id, char * deviceName);

#endif // MOCK_AUTHENTICATOR_H
