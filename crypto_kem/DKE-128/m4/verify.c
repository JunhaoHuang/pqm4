// Comes from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/verify.c

// #include "compat.h"
#include "verify.h"
#if defined(__unix__) || defined(__APPLE__)
    #include <stddef.h>
#endif
#include <stdint.h>

int DKE1_verify(const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;
    uint8_t r = 0;

    for (i = 0; i < len; i++) {
        r |= a[i] ^ b[i];
    }
    
    return (~(uint64_t)r + 1) >> 63; 
}

void DKE1_cmov(uint8_t *r, const uint8_t *x, size_t len, uint8_t b) {
    size_t i;

    // PQCLEAN_PREVENT_BRANCH_HACK(b); // PQClean

    b = -b;
    for (i = 0; i < len; i++) {
        r[i] ^= b & (r[i] ^ x[i]);
    }
}


void DKE1_cmov_int16(int16_t *r, int16_t v, uint16_t b) {
    b = -b;
    *r ^= b & ((*r) ^ v);
}
