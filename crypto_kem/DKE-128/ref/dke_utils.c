#include "ntt.h"
#include "parameters.h"
#include "poly.h"
#include "polyvec.h"
#include "dke_utils.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// TODO: TEST AND VERIFY CONSTANT TIME

//Derived from PQCLEAN_MLKEM512_CLEAN_cmov_int16 in https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-512/clean/verify.c

static void DKE1_cmov_int16(int16_t *r, int16_t v, uint16_t b) {
    /* Copy input v to *r if b is 1, don't modify *r if b is 0.
*              Requires b to be in {0,1};
*              Runs in constant time.
*/
    b = -b;
    *r ^= b & ((*r) ^ v);
}

// Derived from poly_frommsg in https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-512/clean/poly.c
// TODO: TEST. THIS UTILITY IS PROVISIONAL
static void DKE1_rand_to_poly(poly *b, const uint8_t coins[DKE1_N/8]) {
    size_t i, j;
    for (i = 0; i < DKE1_N / 8; i++) {
        for (j = 0; j < 8; j++) {
            b->coeffs[8 * i + j] = 0;
            DKE1_cmov_int16(b->coeffs + 8 * i + j,  -1, (coins[i] >> j) & 1);
        }
    }
}
// TODO: TEST. THIS UTILITY IS PROVISIONAL
void DKE1_signal(uint8_t sig[DKE1_SIGNALBYTES],
                 const poly *k,
                 const uint8_t coins[DKE1_N/8]) {
    poly b;
    DKE1_rand_to_poly(&b, coins);
    DKE1_poly_add(&b, &b, k); // k + b
    DKE1_poly_reduce(&b);         // TODO: can this step be removed and/or improved?
    DKE1_getsignal4(sig, &b);
}

// TODO: TEST. THIS UTILITY IS PROVISIONAL
static void DKE1_apply_signal(poly *k, const uint8_t sig[DKE1_SIGNALBYTES]) {
    poly wL;
    DKE1_poly_fromsignal4(&wL, sig);
    DKE1_poly_sub(k, k, &wL);     // k - wL
    DKE1_poly_reduce(k);              // maps to {-(q-1)/2,...,(q-1)/2}
}



// TODO: TEST. THIS UTILITY IS PROVISIONAL
static void DKE1_mod2(uint8_t ss[DKE1_SSBYTES],  poly *k) {
    unsigned int i, j;
    uint16_t t;
    for (i = 0; i < DKE1_SSBYTES; i++) {
        ss[i] = 0;
        for (j = 0; j < 8; j++) {
            t  = k->coeffs[8 * i + j];
            t &= 1;
            ss[i] |= t << j;
        }
    }
}

// TODO: TEST. THIS UTILITY IS PROVISIONAL
void DKE1_derive_ss(uint8_t ss[DKE1_SSBYTES],
                    poly *k,
                    const uint8_t sig[DKE1_SIGNALBYTES]) {
    DKE1_apply_signal(k, sig);
    DKE1_mod2(ss, k);
}