#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>


int encrypt(uint8_t * payload, unsigned int plength, const uint8_t * key, unsigned int klength);

int decrypt(uint8_t * payload, unsigned int plength, const uint8_t * key, unsigned int klength);

#endif /* CRYPTO_H */
