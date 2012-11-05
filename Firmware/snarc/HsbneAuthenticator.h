#ifndef HSBNE_AUTHENTICATOR_H
#define HSBNE_AUTHENTICATOR_H

/**
 * Authenticatoion providers should have these methods:
 * + void authenticator_init() - Run once at the start to set any required variables/pins/etc.
 * + bool authenticator_authenticate(unsigned long id, char * deviceName) - Run whenever we want to authenticate an ID.
 *
 * This is a mock authenticator for testing purposes. It will accept 0xFFFFFFFF and reject everything else.
 */

#include <SPI.h>
#include <Ethernet.h>
#include "utils.h"

#define AUTH_SERVER_PORT 8001

void authenticator_init();
bool authenticator_authenticate(uint32_t card_id);

#endif // HSBNE_AUTHENTICATOR_H
