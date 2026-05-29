// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-768/clean/ntt.c

#ifndef NTT_H
#define NTT_H

#include <stdint.h>

extern const int32_t zetas[64];

/// @brief Inplace number-theoretic transform (NTT) in Rq = Zq[X]/(x^n+1) where q = 7681 and n = 512,
///        input is in standard order, output is in bitreversed order
/// @param[in,out]  r  pointer to input/output vector of elements of Zq
void DKE1_ntt(int16_t r[256]);


/// @brief Inplace inverse number-theoretic transform in Rq and
///         multiplication by R^2. invntt(r) = NTT^-1(r) R^2 mod q.
///         Input is in bitreversed order, output is in standard order
/// @param[in, out] r   pointer to input/output vector of elements of Zq
void DKE1_invntt(int16_t r[256]);

/// @brief  Pointwise Multiplication of polynomials in Zq[X]/(X^2-zeta)
///         used for multiplication of elements in Rq in NTT domain
/// @param[in]  a       pointer to the first factor
/// @param[in]  b       pointer to the second factor
/// @param[out] r       pointer to the output polynomial
void DKE1_basemul(int16_t r[256], const int16_t a[256], const int16_t b[256]);

/// @brief  Pointwise Multiplication of polynomials in Zq[X]/(X^2-zeta)
///         used for multiplication of elements in Rq in NTT domain
/// @param[in]  a       pointer to the first and accumulate factor
/// @param[in]  b       pointer to the second factor
/// @param[out] r       pointer to the output polynomial
void DKE1_basemul_acc(int16_t r[256], const int16_t a[256], const int16_t b[256]);

#endif
