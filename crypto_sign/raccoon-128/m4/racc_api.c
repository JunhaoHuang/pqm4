//  racc_api.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- NIST KAT Generator API.

#include <string.h>

#include "api.h"
#include "racc_core.h"
#include "racc_serial.h"
#include "xof_sample.h"
#include "fips202.h"
//  Generates a keypair - pk is the public key and sk is the secret key.

int
crypto_sign_keypair(  unsigned char *pk, unsigned char *sk)
{
    bool ret;
#if MEM_OPT == 2
    racc_sk_compress_t r_sk; //  compressed internal-format secret key
    ret = racc_core_keygen(pk, &r_sk); //  generate keypair
#elif MEM_OPT == 1
    racc_sk_t r_sk; //  internal-format secret key
    ret = racc_core_keygen(pk, &r_sk); //  generate keypair
#else
    racc_pk_t r_pk; //  internal-format public key
    racc_sk_t r_sk; //  internal-format secret key

    racc_core_keygen(&r_pk, &r_sk); //  generate keypair

    //  serialize
    ret= (CRYPTO_PUBLICKEYBYTES != racc_encode_pk(pk, &r_pk));
#endif
    memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);
    ret = ret || ((CRYPTO_SECRETKEYBYTES - CRYPTO_PUBLICKEYBYTES) != racc_encode_sk(sk + CRYPTO_PUBLICKEYBYTES, &r_sk));
    if (ret)
        return -1;
    else
        return 0;
}

//  Sign a message: sm is the signed message, m is the original message,
//  and sk is the secret key.

int
crypto_sign(unsigned char *sm, unsigned int *smlen,
            const unsigned char *m, unsigned int mlen,
            const unsigned char *sk)
{
#if MEM_OPT == 2
    racc_sk_compress_t r_sk; //  internal-format compressed secret key
    uint8_t mu[RACC_MU_SZ];
    uint8_t tr[RACC_TR_SZ];
    int ret = 0;
    //  deserialize secret key
    if (CRYPTO_SECRETKEYBYTES != racc_decode_sk(&r_sk, sk))
        return -1;
    // compute pk.tr
    shake256(tr, RACC_TR_SZ, r_sk.pk, CRYPTO_PUBLICKEYBYTES);
    xof_chal_mu(mu, tr, m, mlen); //  compute mu
    memcpy(sm + CRYPTO_BYTES, m, mlen); //  add the message

    ret = racc_core_sign(sm, mu, &r_sk); //  create signature

    
    *smlen = mlen + CRYPTO_BYTES;

    return ret;
#elif MEM_OPT == 1
    racc_sk_t r_sk; //  internal-format secret key
    uint8_t mu[RACC_MU_SZ];
    int ret = 0;

    //  deserialize secret key
    if (CRYPTO_SECRETKEYBYTES != racc_decode_sk(&r_sk, sk))
        return -1;

    xof_chal_mu(mu, r_sk.pk.tr, m, mlen); //  compute mu
    memcpy(sm + CRYPTO_BYTES, m, mlen);   //  add the message

    ret = racc_core_sign(sm, mu, &r_sk); //  create signature

    *smlen = mlen + CRYPTO_BYTES;

    return ret;
#else
    racc_sk_t   r_sk;           //  internal-format secret key
    racc_sig_t  r_sig;          //  internal-format signature
    uint8_t mu[RACC_MU_SZ];
    size_t  sig_sz;
    //  deserialize secret key
    if (CRYPTO_SECRETKEYBYTES != racc_decode_sk(&r_sk, sk))
        return -1;

    xof_chal_mu(mu, r_sk.pk.tr, m, mlen);           //  compute mu
    memcpy(sm + CRYPTO_BYTES, m, mlen);             //  add the message

    //  several trials may be needed in case of signature size overflow
    do {
        racc_core_sign(&r_sig, mu, &r_sk);          //  create signature

        //  The NIST API expects an "envelope" consisting of the message
        //  together with signature. we put the signature first.
        sig_sz = racc_encode_sig(sm, CRYPTO_BYTES, &r_sig);
    } while (sig_sz == 0);

    memset(sm + sig_sz, 0, CRYPTO_BYTES - sig_sz);  //  zero padding

    *smlen = mlen + CRYPTO_BYTES;

    return  0;
#endif
}

//  Verify a message signature: m is the original message, sm is the signed
//  message, pk is the public key.

int
crypto_sign_open(unsigned char *m, unsigned int *mlen,
                 const unsigned char *sm, unsigned int smlen,
                 const unsigned char *pk)
{
    size_t m_sz;
    uint8_t mu[RACC_MU_SZ];
    m_sz = smlen - CRYPTO_BYTES;
#if MEM_OPT > 0
    uint8_t tr[RACC_TR_SZ];

    if (smlen < CRYPTO_BYTES)
        return -1;
    // compute pk.tr
    shake256(tr, RACC_TR_SZ, pk, CRYPTO_PUBLICKEYBYTES);
    // compute mu
    xof_chal_mu(mu, tr, sm + CRYPTO_BYTES, m_sz);

    if (!racc_core_verify(sm, mu, pk))
        return -1;
    
#else
    racc_pk_t   r_pk;           //  internal-format public key
    racc_sig_t  r_sig;          //  internal-format signature

    //  deserialize public key, signature with a consistency check
    if (smlen < CRYPTO_BYTES ||
        CRYPTO_PUBLICKEYBYTES != racc_decode_pk(&r_pk, pk) ||
        CRYPTO_BYTES != racc_decode_sig(&r_sig, sm))
    {
        return -1;
    }

    //  compute mu
    xof_chal_mu(mu, r_pk.tr, sm + CRYPTO_BYTES, m_sz);

    //  verification
    if (!racc_core_verify(&r_sig, mu, &r_pk))
    {
        return -1;
    }
#endif
    
    //  store the length and move the "opened" message
    memcpy(m, sm + CRYPTO_BYTES, m_sz);
    *mlen = m_sz;

    return  0;
}
