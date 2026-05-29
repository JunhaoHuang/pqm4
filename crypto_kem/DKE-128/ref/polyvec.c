// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-512/clean/polyvec.c

#include "parameters.h"
#include "poly.h"
#include "polyvec.h"
#include <stdint.h>

// Basic arithmetic ----------------------------------------------

void DKE1_polyvec_reduce(polyvec *v) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_reduce(&v->vec[i]);
    }
}

void DKE1_polyvec_add(polyvec *res, const polyvec *a, const polyvec *b) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_add(&res->vec[i], &a->vec[i], &b->vec[i]);
    }
}

void DKE1_polyvec_sub(polyvec *res, const polyvec *a, const polyvec *b) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_sub(&res->vec[i], &a->vec[i], &b->vec[i]);
    }
}

void DKE1_polyvec_scale2(polyvec *v) {
    DKE1_polyvec_add(v, v, v);
}

// Advanced arithmetic ---------------------------------------------

void DKE1_polyvec_ntt(polyvec *v) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_ntt(&v->vec[i]);
    }
}

void DKE1_polyvec_invntt_tomont(polyvec *v) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_invntt_tomont(&v->vec[i]);
    }
}

void DKE1_polyvec_basemul_acc_montgomery(poly *res, const polyvec *a, const polyvec *b) {
    unsigned int i;
    // the sums are accumulated in res
    poly temp;           // auxiliary poly to contain the successive products
    DKE1_poly_basemul_montgomery(res, &a->vec[0], &b->vec[0]);
    for (i = 1; i < DKE1_K; i++) {
        DKE1_poly_basemul_montgomery(&temp, &a->vec[i], &b->vec[i]);
        DKE1_poly_add(res, res, &temp);
    }
    DKE1_poly_reduce(res);
}

// For managing conversion polyvec < --- > bytes ----------------------------

void DKE1_polyvec_tobytes(uint8_t bytes[DKE1_POLYVECBYTES], const polyvec *v) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_tobytes(bytes +  i * DKE1_POLYBYTES, &v->vec[i]);
    }
}

void DKE1_polyvec_frombytes(polyvec *v, const uint8_t bytes[DKE1_POLYVECBYTES]) {
    unsigned int i;
    for (i = 0; i < DKE1_K; i++) {
        DKE1_poly_frombytes(&v -> vec[i], bytes + i * DKE1_POLYBYTES);
    }
}

void DKE1_polyvec_compress10(uint8_t bytes[DKE1_PBCOMPRESSEDBYTES], const polyvec *v) {
    unsigned int i, j, k;
    uint64_t d0;        // 64 bits = 8 bytes
    uint16_t t[4];
    for (i = 0; i < DKE1_K; i++) {
        for (j = 0; j < DKE1_N/4; j++) {    // we process 4 coefficients each time
            for (k = 0; k < 4; k++) {           // for each coefficient...
                t[k] = v->vec[i].coeffs[4*j+k];
                t[k] += ((int16_t)t[k] >> 15) & DKE1_Q; // branch-free map to {0,...,q-1}
                // Compress from Zq --> Z2^d definition: round(x · 2^d / q) = floor(x · 2^d / q + 0.5)
                d0 = t[k];      // x
                d0 <<= 10;      // x · 2^d
                d0 += 1665;     // x · 2^d + (q/2+0.5)
                d0 *= 1290167;  // 1290167 = floor(2^32 / q) => d0 = (x·2^d + q/2 + 0.5)·floor(2^32/q)
                d0 >>= 32;      // d0 = floor((x·2^d/q + 0.5)/q)
                t[k] = d0 & 0x3ff; // take the last 10 bits.
            }
            // At this stage, there are 4 coefficients in t
            // We use 5 bytes to store them
            bytes[0] = (uint8_t)(t[0] >> 0);
            bytes[1] = (uint8_t)((t[0] >> 8) | (t[1] << 2));
            bytes[2] = (uint8_t)((t[1] >> 6) | (t[2] << 4));
            bytes[3] = (uint8_t)((t[2] >> 4) | (t[3] << 6));
            bytes[4] = (uint8_t)(t[3] >> 2);

            bytes += 5;     // move to next position
        }
    }
}

void DKE1_polyvec_decompress10(polyvec *v, const uint8_t bytes[DKE1_PBCOMPRESSEDBYTES]) {
    unsigned int i, j, k;

    uint16_t t[4];
    for (i = 0; i < DKE1_K; i++) {
        for (j = 0; j < DKE1_N / 4; j++) {
            // Bit unpacking
            t[0] = (bytes[0] >> 0) | ((uint16_t)bytes[1] << 8);
            t[1] = (bytes[1] >> 2) | ((uint16_t)bytes[2] << 6);
            t[2] = (bytes[2] >> 4) | ((uint16_t)bytes[3] << 4);
            t[3] = (bytes[3] >> 6) | ((uint16_t)bytes[4] << 2);
            bytes += 5;

            // Round(t q / 2^10)
            for (k = 0; k < 4; k++) {
                v->vec[i].coeffs[4 * j + k] = ((uint32_t)(t[k] & 0x3FF) * DKE1_Q + 512) >> 10;
            }
        }
    }
}
