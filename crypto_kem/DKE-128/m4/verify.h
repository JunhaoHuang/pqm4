#ifndef VERIFY_H
#define VERIFY_H


// Comes from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/verify.h

#ifndef DKE_VERIFY_H
#define DKE_VERIFY_H
#include <stddef.h>
#include <stdint.h>

int DKE1_verify(const uint8_t *a, const uint8_t *b, size_t len);

void DKE1_cmov(uint8_t *r, const uint8_t *x, size_t len, uint8_t b);

void DKE1_cmov_int16(int16_t *r, int16_t v, uint16_t b);

#endif

#endif //VERIFY_H
