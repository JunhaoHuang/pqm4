#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fips202.h"
#include "param.h"
#include "pke.h"
#include "code.h"
#include "randombytes.h"
#include "api.h"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
    uint8_t f2[SKF2LEN], k[MESSLEN];
    int16_t tmp_pk[DIM_N], tmp_f[DIM_N];
    uint8_t seed[SEEDLEN];
    randombytes(seed, SEEDLEN);
    for (int i = 0; i < SEEDLEN; i++)
    {
        seed[i] = 0;
    }
    PKE_KeyGen(seed, tmp_pk, tmp_f, f2, k);

    encode_pk(tmp_pk, pk);

    // encode sk = pk || k || f || f2 || H(pk)
    memcpy(sk, pk, PKLEN);                                        // pk
    memcpy(sk + PKLEN, k, MESSLEN);                               // k
    encode_f(tmp_f, sk + PKLEN + MESSLEN);                        // SKF
    memcpy(sk + PKLEN + MESSLEN + SKFLEN, f2, SKF2LEN);           // skf2
    sha3_256(sk + PKLEN + MESSLEN + SKFLEN + SKF2LEN, pk, PKLEN); // H(pk)
    return 0;
}

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk)
{
    uint8_t Kp[PKLEN], m[MESSLEN + SYMBYTES];
    int16_t tmp_pk[DIM_N];
    randombytes(m, MESSLEN);
    memcpy(Kp, pk, PKLEN);
    sha3_256(m + MESSLEN, Kp, PKLEN);
    sha3_512(Kp, m, MESSLEN + SYMBYTES);
    decode_pk(pk, tmp_pk);
    PKE_Encrypt(ct, tmp_pk, m, Kp + SYMBYTES);
    memcpy(Kp + SYMBYTES, ct, CTLEN);
    sha3_256(ss, Kp, SYMBYTES + CTLEN);
    return 0;
}

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk)
{
    // sk = pk || k || f || f2 || H(pk)
    uint8_t Kp[PKLEN + SYMBYTES], m[MESSLEN + SYMBYTES], cp[CTLEN];
    int16_t tmp_pk[DIM_N], tmp_f[DIM_N];
    uint8_t f2[SKF2LEN], f[SKFLEN], pk[PKLEN], k[MESSLEN];

    memcpy(pk, sk, PKLEN);
    memcpy(k, sk + PKLEN, MESSLEN);
    memcpy(f, sk + PKLEN + MESSLEN, SKFLEN);
    memcpy(f2, sk + PKLEN + MESSLEN + SKFLEN, SKF2LEN);

    decode_f(f, tmp_f);
    PKE_Decrypt(ct, tmp_f, f2, m);
    memcpy(m + MESSLEN, sk + PKLEN + MESSLEN + SKFLEN + SKF2LEN, SYMBYTES); // H(pk)
    sha3_512(Kp, m, MESSLEN + SYMBYTES);
    decode_pk(pk, tmp_pk); // pk
    PKE_Encrypt(cp, tmp_pk, m, Kp + SYMBYTES);

    if (memcmp(ct, cp, CTLEN) != 0)
    {
        memcpy(Kp, k, MESSLEN);
        memcpy(Kp + SYMBYTES, ct, CTLEN);
        sha3_256(ss, Kp, SYMBYTES + CTLEN);
    }
    else
    {
        memcpy(Kp + SYMBYTES, ct, CTLEN);
        sha3_256(ss, Kp, SYMBYTES + CTLEN);
    }
    return 0;
}
