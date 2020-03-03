#include <errno.h>

#include "crypto.h"


int encrypt(uint8_t * payload, unsigned int plength, const uint8_t * key, unsigned int klength)
{
  if (klength < plength) {
#ifndef _WIN32
      errno = EKEYREJECTED;
#endif // !_WIN32
    return -1;
  }
  for (unsigned int i = 0u; i < plength; ++i) {
    *(payload + i) ^= *(key + i);
  }
  return 0;
}

int decrypt(uint8_t * payload, unsigned int plength, const uint8_t * key, unsigned int klength)
{
  return encrypt(payload, plength, key, klength);
}
