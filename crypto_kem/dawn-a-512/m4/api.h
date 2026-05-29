#ifndef API_H
#define API_H

#include "param.h"

#define CRYPTO_SECRETKEYBYTES  (PKLEN+SKFLEN+SKF2LEN+SYMBYTES+MESSLEN)
#define CRYPTO_PUBLICKEYBYTES  PKLEN
#define CRYPTO_CIPHERTEXTBYTES CTLEN
#define CRYPTO_BYTES           SYMBYTES
#define M4
#define CRYPTO_ALGNAME "Dawn-A-512"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);


#endif
