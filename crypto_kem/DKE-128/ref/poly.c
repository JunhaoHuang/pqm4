// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/poly.c

#include "parameters.h"
#include "poly.h"
#include "reduce.h"
#include "ntt.h"
#include <stdint.h>
#include <stddef.h>

// Basic arithmetic ----------------------------------------------

void DKE1_poly_reduce(poly *pol) {
    unsigned int i;
    for (i = 0; i < DKE1_N; i++) {
        pol->coeffs[i] = DKE1_barrett_reduce(pol->coeffs[i]);
    }
}

void DKE1_poly_add(poly *r, const poly *a, const poly *b) {
    unsigned int i;
    for (i = 0; i < DKE1_N; i++) {
        r->coeffs[i] = a->coeffs[i] + b->coeffs[i];
    }
}

void DKE1_poly_sub(poly *r, const poly *a, const poly *b) {
    unsigned int i;
    for (i = 0; i < DKE1_N; i++) {
        r->coeffs[i] = a->coeffs[i] - b->coeffs[i];
    }
}

void DKE1_poly_scale2(poly *pol) {
    DKE1_poly_add(pol, pol, pol);
}

// Advanced arithmetic -----------------------------------


void DKE1_poly_ntt(poly *pol) {
    DKE1_ntt(pol->coeffs);  // Apply NTT.
    DKE1_poly_reduce(pol);  // Barret reduction
}

void DKE1_poly_invntt_tomont(poly *pol) {
    DKE1_invntt(pol->coeffs);   // NTT & Montgomery Domain -> Montgomery Domain
}

void DKE1_poly_basemul_montgomery(poly *res, const poly *a, const poly *b) {
    unsigned int i;
    for (i = 0; i < DKE1_N / 4; i++) {
        DKE1_basemul(&res->coeffs[4 * i], &a->coeffs[4 * i], &b->coeffs[4 * i], DKE1_zetas[64 + i]);
        DKE1_basemul(&res->coeffs[4 * i + 2], &a->coeffs[4 * i + 2], &b->coeffs[4 * i + 2], -DKE1_zetas[64 + i]);
    }
} // NTT & Montgomery Domain -> NTT & Montgomery Domain

void DKE1_poly_tomont(poly *pol){   // -> Montgomery Domain
    unsigned int i;
    const int16_t f = (1ULL << 32) % DKE1_Q;
    for (i = 0; i < DKE1_N; i++) {
        pol->coeffs[i] = DKE1_montgomery_reduce((int32_t)pol->coeffs[i] * f);
    }
}

// For managing conversion poly <---> bytes ----------------------------

void DKE1_poly_tobytes(uint8_t bytes[DKE1_POLYBYTES], const poly *pol){
    unsigned int i;
    uint16_t t0, t1;

    for (i = 0; i < DKE1_N / 2; i++) {
        // map to positive standard representatives
        t0  = pol->coeffs[2 * i];
        t0 += ((int16_t)t0 >> 15) & DKE1_Q;
        t1 = pol->coeffs[2 * i + 1];
        t1 += ((int16_t)t1 >> 15) & DKE1_Q;
        // We use 3 bytes to store two coefficients
        bytes[3 * i + 0] = (uint8_t)(t0 >> 0);
        bytes[3 * i + 1] = (uint8_t)((t0 >> 8) | (t1 << 4));
        bytes[3 * i + 2] = (uint8_t)(t1 >> 4);
    }
}

void DKE1_poly_frombytes(poly *pol, const uint8_t bytes[DKE1_POLYBYTES]) {
    unsigned int i;
    for (i = 0; i < DKE1_N / 2; i++) {
        // We get 2 coefficients from 3 bytes
        pol->coeffs[2 * i]   = ((bytes[3 * i + 0] >> 0) | ((uint16_t)bytes[3 * i + 1] << 8)) & 0xFFF;
        pol->coeffs[2 * i + 1] = ((bytes[3 * i + 1] >> 4) | ((uint16_t)bytes[3 * i + 2] << 4)) & 0xFFF;
    }
}

// We use PQCLean MLKEM512 compression for implementing DKE1_getsignal4 (l=4)
// This is not valid for other choices of l.
static void PQCLEAN_MLKEM512_CLEAN_poly_compress(uint8_t bytes[DKE1_SIGNALBYTES], const poly *a) {
    unsigned int i, j;
    int16_t u;
    uint32_t d0;
    uint8_t t[8];
    for (i = 0; i < DKE1_N / 8; i++) {
        for (j = 0; j < 8; j++) {
            // map to positive standard representatives
            u  = a->coeffs[8 * i + j];
            u += (u >> 15) & DKE1_Q;
            /*    t[j] = ((((uint16_t)u << 4) + KYBER_Q/2)/KYBER_Q) & 15; */
            d0 = u << 4;
            d0 += 1665;
            d0 *= 80635;
            d0 >>= 28;
            t[j] = d0 & 0xf;
        }
        bytes[0] = t[0] | (t[1] << 4);
        bytes[1] = t[2] | (t[3] << 4);
        bytes[2] = t[4] | (t[5] << 4);
        bytes[3] = t[6] | (t[7] << 4);
        bytes += 4;
    }
}

void DKE1_getsignal4(uint8_t sig[DKE1_SIGNALBYTES], const poly *pol) {
    PQCLEAN_MLKEM512_CLEAN_poly_compress(sig, pol);
}

// -------------------------------------------------------------------------------
// One option for implementing DKE1_poly_fromsignal4: to use PQCLean MLKEM512 decompression
// This is not valid for other choices of l.
/// @WARNING This does not exactly implement DKE's mapping w -> wL. For w = 7,...,15, decompress(w) = wL +1
static void PQCLEAN_MLKEM512_CLEAN_poly_decompress(poly *r, const uint8_t a[DKE1_SIGNALBYTES]) {
    unsigned int i;

    for (i = 0; i < DKE1_N / 2; i++) {
        r->coeffs[2 * i + 0] = (((uint16_t)(a[0] & 15) * DKE1_Q) + 8) >> 4; // 15 = 0b00001111
        r->coeffs[2 * i + 1] = (((uint16_t)(a[0] >> 4) * DKE1_Q) + 8) >> 4;
        a += 1;
    }
}

// Another option: slightly modify the previous function. (Substitute round(x q /2^4) for floor(x q /2^4))
// TODO: test. Specially for constant time behaviour
static void DKE1_poly_decompressFloor(poly *r, const uint8_t a[DKE1_SIGNALBYTES]){
    unsigned int i;
    for (i = 0; i < DKE1_N / 2; i++) {
        r->coeffs[2 * i + 0] = ((uint16_t)(a[0] & 15) * DKE1_Q) >> 4; // 15 = 0b00001111
        r->coeffs[2 * i + 1] = ((uint16_t)(a[0] >> 4) * DKE1_Q) >> 4;
        a += 1;
    }
}

void DKE1_poly_fromsignal4(poly *pol, const uint8_t sig[DKE1_SIGNALBYTES]) {
    // PQCLEAN_MLKEM512_CLEAN_poly_decompress(pol, sig);
    DKE1_poly_decompressFloor(pol, sig);
}