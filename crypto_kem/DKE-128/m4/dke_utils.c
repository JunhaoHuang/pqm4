#include "ntt.h"
#include "parameters.h"
#include "poly.h"
#include "polyvec.h"
#include "dke_utils.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>


extern void rand_to_poly_asm(int16_t *coeffs, const uint8_t *coins);
extern void mod2_asm(uint8_t *ss, const int16_t *coeffs);
// TODO: TEST. THIS UTILITY IS PROVISIONAL
static void DKE1_rand_to_poly(poly *b, const uint8_t coins[DKE1_N/8]) {
    rand_to_poly_asm(b->coeffs, coins);
}

// TODO: TEST. THIS UTILITY IS PROVISIONAL
void DKE1_signal(uint8_t sig[DKE1_SIGNALBYTES],
                 const poly *k,
                 const uint8_t coins[DKE1_N/8]) {
    poly b;
    DKE1_rand_to_poly(&b, coins);
    DKE1_poly_add(&b, &b, k); // k + b
    DKE1_poly_reduce_mq(&b);         // TODO: can this step be removed and/or improved?
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
    mod2_asm(ss, k->coeffs);
}

// TODO: TEST. THIS UTILITY IS PROVISIONAL
void DKE1_derive_ss(uint8_t ss[DKE1_SSBYTES],
                    poly *k,
                    const uint8_t sig[DKE1_SIGNALBYTES]) {
    DKE1_apply_signal(k, sig);
    DKE1_mod2(ss, k);
}
