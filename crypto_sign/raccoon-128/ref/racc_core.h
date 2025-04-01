//  racc_core.h
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- Core internal API.

#ifndef _RACC_CORE_H_
#define _RACC_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mask_random.h"
#include "racc_param.h"

//  === Global namespace prefix
#ifdef RACC_
#define racc_core_keygen RACC_(core_keygen)
#define racc_core_sign RACC_(core_sign)
#define racc_core_verify RACC_(core_verify)
#endif

//  === Internal structures ===

//  raccoon public key
typedef struct {
    uint8_t a_seed[RACC_AS_SZ];             //  seed for a
    int64_t t[RACC_K][RACC_N];              //  public key
    uint8_t tr[RACC_TR_SZ];                 //  hash of serialized public key
} racc_pk_t;

//  raccoon secret key
typedef struct {
    racc_pk_t pk;                           //  copy of public key
    int64_t s[RACC_ELL][RACC_D][RACC_N];    //  d-masked secret key
} racc_sk_t;

//  raccoon signature
typedef struct {
    uint8_t ch[RACC_CH_SZ];                 //  challenge hash
    int64_t h[RACC_K][RACC_N];              //  hint
    int64_t z[RACC_ELL][RACC_N];            //  signature data
} racc_sig_t;

//  === Core API ===

//  Generate a public-secret keypair ("pk", "sk").
void racc_core_keygen(racc_pk_t *pk, racc_sk_t *sk);

//  Create a detached signature "sig" for digest "mu" using secret key "sk".
void racc_core_sign(racc_sig_t *sig, const uint8_t mu[RACC_MU_SZ],
                    racc_sk_t *sk);

//  Verify that the signature "sig" is valid for digest "mu".
//  Returns true iff signature is valid, false if not valid.
bool racc_core_verify(  const racc_sig_t *sig,
                        const uint8_t mu[RACC_MU_SZ],
                        const racc_pk_t *pk);

void racc_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N]);
void racc_ntt_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N]);
void zero_encoding(int64_t z[RACC_D][RACC_N], mask_random_t *mrg);
void racc_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg);
void racc_ntt_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg);
void add_rep_noise(int64_t vi[RACC_D][RACC_N],
                   int i_v, int u, mask_random_t *mrg);
#ifdef __cplusplus
}
#endif

//  _RACC_CORE_H_
#endif
