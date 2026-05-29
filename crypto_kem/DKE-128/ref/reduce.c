// Comes from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/reduce.c

#include "parameters.h"
#include "reduce.h"
#include <stdint.h>

int16_t DKE1_montgomery_reduce(int32_t a) {
    int16_t t;

    t = (int16_t)a * QINV;
    t = (a - (int32_t)t * DKE1_Q) >> 16;
    return t;
}

int16_t DKE1_barrett_reduce(int16_t a) {
    int16_t t;
    const int16_t v = ((1 << 26) + DKE1_Q / 2) / DKE1_Q;

    t  = ((int32_t)v * a + (1 << 25)) >> 26;
    t *= DKE1_Q;
    return a - t;
}