// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/ntt.c

#ifndef NTT_H
#define NTT_H

#include <stdint.h>

extern const int16_t DKE1_zetas[128];

/// @brief Inplace number-theoretic transform (NTT) in Rq = Zq[X]/(x^n+1) where q = 3329 and n = 256,
///         multiplied by Montgomery factor R^-1. i.e. ntt(r) = NTT(r)R^-1 mod q
///        input is in standard order, output is in bitreversed order
/// @param[in,out]  r  pointer to input/output vector of elements of Zq
void DKE1_ntt(int16_t r[256]);


/// @brief Inplace inverse number-theoretic transform in Rq and
///         multiplication by R^2. invntt(r) = NTT^-1(r) R^2 mod q.
///         Input is in bitreversed order, output is in standard order
/// @param[in, out] r   pointer to input/output vector of elements of Zq
void DKE1_invntt(int16_t r[256]);

/// @brief  Multiplication of polynomials in Zq[X]/(X^2-zeta)
///         used for multiplication of elements in Rq in NTT domain
/// @param[in]  a       pointer to the first factor
/// @param[in]  b       pointer to the second factor
/// @param[in]  zeta    integer defining the reduction polynomial
/// @param[out] r       pointer to the output polynomial
void DKE1_basemul(int16_t r[2], const int16_t a[2], const int16_t b[2], int16_t zeta);

#endif
