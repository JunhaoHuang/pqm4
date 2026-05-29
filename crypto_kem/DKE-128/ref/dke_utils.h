#ifndef DKE_UTILS_H
#define DKE_UTILS_H

#include "parameters.h"
#include "poly.h"
#include <string.h>
#include <stdint.h>


/// @brief Computes the DKE1 signal function for the polynomial k using coins as randomness
/// @param[out] sig     pointer to output byte array
/// @param[in]  k       pointer to input polynomial
/// @param[in]  coins   pointer to input randomness
void DKE1_signal(uint8_t sig[DKE1_SIGNALBYTES],
                 const poly *k,
                 const uint8_t coins[DKE1_N/8]);

/// @brief Reconciliation function. Derives ss from poly k and signal sig.
/// @param[out] ss      pointer to output shared secret byte array
/// @param[in]  k       pointer to input polynomial
/// @param[in]  sig     pointer to input signal byte array
void DKE1_derive_ss(uint8_t ss[DKE1_SSBYTES],
                    poly *k,
                    const uint8_t sig[DKE1_SIGNALBYTES]);

#endif //DKE_UTILS_H
