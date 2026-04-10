#ifndef PKE_H
#define PKE_H

#include <stdint.h>

void PKE_KeyGen(uint8_t *seed, int16_t *pk, int16_t *f, uint8_t *f2, uint8_t *k);
void PKE_Encrypt(uint8_t *c, int16_t *pk, uint8_t *m, uint8_t *seed);
void PKE_Decrypt(const uint8_t *c, int16_t *f, uint8_t *f2, uint8_t *m);

#endif