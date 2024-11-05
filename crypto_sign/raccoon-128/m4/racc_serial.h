//  racc_serial.h
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- Serialize/deserialize.

#ifndef _RACC_SERIAL_H_
#define _RACC_SERIAL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "racc_param.h"

//  === Global namespace prefix

#ifdef RACC_
#define racc_encode_pk RACC_(encode_pk)
#define racc_decode_pk RACC_(decode_pk)
#define racc_encode_sk RACC_(encode_sk)
#define racc_decode_sk RACC_(decode_sk)
#define racc_encode_sig RACC_(encode_sig)
#define racc_decode_sig RACC_(decode_sig)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//  Encode public key "pk" to bytes "b". Return length in bytes.
size_t racc_encode_pk(uint8_t *b, const racc_pk_t *pk);

//  Decode a public key from "b" to "pk". Return length in bytes.
size_t racc_decode_pk(racc_pk_t *pk, const uint8_t *b);


//  Encode signature "sig" to "*b" of max "b_sz" bytes. Return length in
//  bytes or zero in case of overflow.
#if MEM_OPT ==2  // on-the-fly decode/encode; encode/decode h before z.
size_t racc_encode_sig_h(uint8_t *b, size_t i, size_t b_sz, size_t l_h, uint8_t *pre_z, size_t *pre_k, const int64_t sig_h[RACC_N]);
size_t racc_encode_sig_z(uint8_t *b, size_t b_sz, size_t l_z, uint8_t *pre_z, size_t *pre_k, const int64_t sig_z[RACC_N]);
size_t racc_decode_sig_z(int64_t sig_z[RACC_N], size_t b_sz, size_t l_z, uint8_t *pre_z, size_t *pre_k, const uint8_t *b);
#elif MEM_OPT == 1
size_t racc_encode_sig_zh(uint8_t *b, size_t b_sz, const int64_t h[RACC_K][RACC_N], const int64_t z[RACC_ELL][RACC_N]);
#else
size_t racc_encode_sig(uint8_t *b, size_t b_sz, const racc_sig_t *sig);
#endif

	//  decode bytes "b" into signature "sig". Return length in bytes.
	size_t racc_decode_sig(racc_sig_t *sig, const uint8_t *b);

#if MEM_OPT > 0
// on-the-fly version of the above functions for memory optimizations
size_t racc_encode_sk_l(uint8_t *b, const int64_t s[RACC_D][RACC_N], size_t i, size_t lk, size_t ls);
size_t racc_decode_sk_l(int64_t sk[RACC_D][RACC_N], const uint8_t *b, size_t i, size_t lk, size_t ls);

size_t racc_encode_pk_k(uint8_t *b, const int64_t *t, size_t l);
size_t racc_decode_pk_k(int64_t pk[RACC_N], const uint8_t *b, size_t l);
#else
//  Encode secret key "sk" to bytes "b". Return length in bytes.
size_t racc_encode_sk(uint8_t *b, const racc_sk_t *sk);

//  Decode a secret key from "b" to "sk". Return length in bytes.
size_t racc_decode_sk(racc_sk_t *sk, const uint8_t *b);

#endif

#ifdef __cplusplus
}
#endif

//  _RACC_SERIAL_H_
#endif
