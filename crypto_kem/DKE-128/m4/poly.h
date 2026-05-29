#ifndef POLY_H
#define POLY_H
// Derived from https://github.com/PQClean/PQClean/blob/master/crypto_kem/ml-kem-512/clean/poly.h

#include "parameters.h"
#include <stdint.h>

// This file defines the representation of elements in Rq and its arithmetic

typedef struct {
    int16_t coeffs[DKE1_N];
} poly;             // Represents polynomials


// Basic arithmetic -------------------------------------------------------

/// @brief reduces a polynomial mod q in {-(q-1)/2,...,(q-1)/2}
/// @param[in/out] pol pointer to poly pol
void DKE1_poly_reduce(poly *pol);

/// @brief reduces a polynomial mod q in {0,...,q-1}
/// @param[in/out] pol pointer to poly pol
void DKE1_poly_reduce_mq(poly *pol);

/// @brief Adds two polynomials in Rq
/// @param[in] a        pointer to a poly a
/// @param[in] b        pointer to a poly b
/// @param[out] res     pointer to output poly res = a + b (might not be in Rq)
void DKE1_poly_add(poly *res, const poly *a, const poly *b);
void DKE1_poly_sub(poly *res, const poly *a, const poly *b);

/// @brief Scales a polynomial by a factor of 2
/// @param[in, out] pol  pointer to a poly
/// @remark If pol has small coefficients, this new polynomial belongs to Rq
void DKE1_poly_scale2(poly *pol);

// Advanced arithmetic ----------------------------------------------------

/// @brief Computes the Number-Theoretic Transform (NTT) of a polynomial
/// Transforms the polynomial from coefficient representation to NTT
/// representation. poly_ntt(f) = NTT(f) mod q
/// @param[in,out]      pol Pointer to the polynomial to be transformed
void DKE1_poly_ntt(poly *pol);

/// @brief Computes the inverse Number-Theoretic Transform (INTT) and
/// exits the Plantard-backed NTT domain into coefficient representation
/// @param[in,out]      pol Pointer to the polynomial to be transformed
void DKE1_poly_invntt(poly *pol);

/// @brief Multiplies two polynomials in the Plantard-backed NTT domain
///        coefficient-wise.
/// @param[out]     res  Pointer to output polynomial in NTT representation
/// @param[in]      a  Pointer to input polynomial a in NTT representation
/// @param[in]      b  Pointer to input polynomial b in NTT representation
void DKE1_poly_basemul(poly *res, const poly *a, const poly *b);

/// @brief Multiplies two polynomials in the Plantard-backed NTT domain
///        coefficient-wise.
/// @param[out]     res  Pointer to accumulate and output polynomial in NTT representation
/// @param[in]      a  Pointer to input polynomial a in NTT representation
/// @param[in]      b  Pointer to input polynomial b in NTT representation
void DKE1_poly_basemul_acc(poly *res, const poly *a, const poly *b);


/// @brief Converts a polynomial from Plantard domain back to coefficient representation.
/// @param[in,out]      pol Pointer to polynomial to be converted
void DKE1_poly_fromplant(poly *pol);

// For managing conversion poly < --- > bytes ----------------------------

/// @brief Serialize a polynomial (without compressing).
/// @param[in] pol      pointer to input polynomial
/// @param[out] bytes   pointer to the beginning of the byte array
void DKE1_poly_tobytes(uint8_t bytes[DKE1_POLYBYTES], const poly *pol);


/// @brief De-serialize a polynomial (without decompressing) and multiply with polynomial b.
/// @param[out] pol      pointer to output polynomial
/// @param[out] b        pointer to input polynomial
/// @param[in]  bytes     pointer to the beginning of the byte array
void DKE1_poly_frombytes_mul(poly *pol, const poly *b, const unsigned char *bytes);

/// @brief Multiplication of a polynomial with a de-serialization of another polynomial Accumulation in r.
 /// @param[out] pol      pointer to output polynomial
 /// @param[out] b        pointer to input polynomial
 /// @param[in]  bytes     pointer to the beginning of the byte array
 void DKE1_poly_frombytes_mul_acc(poly *pol, const poly *b, const unsigned char *bytes);

/// @brief Evaluates the (derandomized) signal function (with parameter DKE_L=4)  from poly.
/// @param[out] sig     array of signal bytes
/// @param[in]  pol     pointer to input polynomial
/// @WARNING    This implementation is **fixed for DKE_L = 4** (matching MLKEM parameters) and optimized accordingly.
///             Changing DKE_L would require rewriting this function, as the current
///             code assumes this specific value for efficiency.
void DKE1_getsignal4(uint8_t sig[DKE1_SIGNALBYTES], const poly *pol);


/// @brief Reconstructs a polynomial from signal: pol = L · (w0 + w1·X + ... + w_255 x^255 )
/// where L = (q-1)/2^l.
/// @param[out] pol     pointer to output polynomial
/// @param[in]  sig     input signal array
void DKE1_poly_fromsignal4(poly *pol, const uint8_t sig[DKE1_SIGNALBYTES]);

#endif //POLY_H
