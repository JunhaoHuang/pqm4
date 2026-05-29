#ifndef DKECCA_H
#define DKECCA_H

#include "parameters.h"
#include <stdint.h>



/// @brief Deterministically derives from coins a public and secret key for DKE(CCA)1
/// @param[in]  coins random coins
/// @param[out] pk    public key
/// @param[out] sk    secret key
void DKE1CCA_keygen_derand(uint8_t pk[DKE1_PKBYTES],
                          uint8_t sk[DKE1_SKBYTES],
                          const uint8_t coins[DKE1_SEEDBYTES + DKE1_SSBYTES]);

/// @brief Deterministically derives a ciphertext and shared secret from the public
///        key and random coins for DKE(CCA)1 (CPA ct | tag)
/// @param[out] ct      pointer to output ciphertext
/// @param[out] ss      pointer to output shared key
/// @param[in]  pk      pointer to input public key
/// @param[in]  coins   pointer to input random coins
void DKE1CCA_enc_derand(uint8_t ct[DKE1_CTBYTES],
                        uint8_t ss[DKE1_SSBYTES],
                        const uint8_t pk[DKE1_PKBYTES],
                        const uint8_t coins[DKE1_SEEDBYTES]);

/// @brief Deterministically derives a shared secret from the ciphertext
///        and own private key. Performs implicit rejection
/// @param[out] ss      pointer to output shared key
/// @param[in]  sk      pointer to input secret key
/// @param[in]  ct      pointer to input ciphertext
void DKE1CCA_dec(uint8_t ss[DKE1_SSBYTES],
                 const uint8_t sk[DKE1_SKBYTES],
                 const uint8_t ct[DKE1_CTBYTES]);

#endif //DKECCA_H
