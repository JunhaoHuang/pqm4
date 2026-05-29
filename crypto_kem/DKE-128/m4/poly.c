// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/poly.c

#include "parameters.h"
#include "poly.h"
#include "ntt.h"
#include <stdint.h>
#include <stddef.h>

// assembly routines
extern void poly_reduce_asm(int16_t* r);
// extern void asm_barrett_reduce(int16_t *r);
extern void poly_reduce_mq_asm(int16_t *r);
extern void asm_fromplant(int16_t *r);
extern void pointwise_add(int16_t *, const int16_t *, const int16_t *);
extern void pointwise_sub(int16_t *, const int16_t *, const int16_t *);
extern void frombytes_mul_asm_acc(int16_t *r, const int16_t *b, const unsigned char *c, const int32_t zetas[64]);
extern void frombytes_mul_asm(int16_t *r, const int16_t *b, const unsigned char *c, const int32_t zetas[64]);
extern void poly_tobytes_asm(uint8_t *bytes, const int16_t *coeffs);
extern void poly_decompress_floor_asm(int16_t *coeffs, const uint8_t *bytes);

// Basic arithmetic ----------------------------------------------
// output (-q/2,q/2)
void DKE1_poly_reduce(poly *pol) {
    poly_reduce_asm(pol->coeffs);
}

void DKE1_poly_reduce_mq(poly *pol){
    poly_reduce_mq_asm(pol->coeffs);
}


void DKE1_poly_add(poly *r, const poly *a, const poly *b) {
    pointwise_add(r->coeffs, a->coeffs, b->coeffs);
}

void DKE1_poly_sub(poly *r, const poly *a, const poly *b) {
    pointwise_sub(r->coeffs, a->coeffs, b->coeffs);
}


void DKE1_poly_scale2(poly *pol) {
    pointwise_add(pol->coeffs, pol->coeffs, pol->coeffs);
}

// Advanced arithmetic -----------------------------------


void DKE1_poly_ntt(poly *pol) {
    DKE1_ntt(pol->coeffs);
}

void DKE1_poly_invntt(poly *pol) {
    DKE1_invntt(pol->coeffs);
}

void DKE1_poly_basemul(poly *res, const poly *a, const poly *b) {
    DKE1_basemul(res->coeffs, a->coeffs, b->coeffs);
} // NTT & Plantard Domain -> NTT & Plantard Domain

void DKE1_poly_basemul_acc(poly *res, const poly *a, const poly *b)
{
    DKE1_basemul_acc(res->coeffs, a->coeffs, b->coeffs);
} // NTT & Plantard Domain -> NTT & Plantard Domain


void DKE1_poly_fromplant(poly *pol) {
    asm_fromplant(pol->coeffs);
}


// For managing conversion poly <---> bytes ----------------------------

void DKE1_poly_tobytes(uint8_t bytes[DKE1_POLYBYTES], const poly *pol){
    poly_tobytes_asm(bytes, pol->coeffs);
}

/*************************************************
 * Name:        poly_frombytes_mul
 *
 * Description: Multiplication of a polynomial with a de-serialization of another polynomial
 *
 * Arguments:   - poly *r:                pointer to output polynomial
 *              - const poly *b:          pointer to input polynomial
 *              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
 **************************************************/

void DKE1_poly_frombytes_mul(poly *r, const poly *b, const unsigned char *a)
{
    frombytes_mul_asm(r->coeffs, b->coeffs, a, zetas);
}

/*************************************************
 * Name:        poly_frombytes_mul_acc
 *
 * Description: Multiplication of a polynomial with a de-serialization of another polynomial
 *              Accumulation in r.
 *
 * Arguments:   - poly *r:                pointer to output polynomial
 *              - const poly *b:          pointer to input polynomial
 *              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
 **************************************************/

void DKE1_poly_frombytes_mul_acc(poly *r, const poly *b, const unsigned char *a)
{
    frombytes_mul_asm_acc(r->coeffs, b->coeffs, a, zetas);
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
            u  = a->coeffs[8 * i + j];
            /*t[j] = ((((uint16_t)u << 4) + KYBER_Q/2)/KYBER_Q) & 15; */
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

void DKE1_poly_fromsignal4(poly *pol, const uint8_t sig[DKE1_SIGNALBYTES]) {
    poly_decompress_floor_asm(pol->coeffs, sig);
    // DKE1_poly_decompressFloor(pol, sig);
}
