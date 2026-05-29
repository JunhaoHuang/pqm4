/*
The software is provided by the Institute of Commercial Cryptography Standards
(ICCS), and is used for algorithm submissions in the Next-generation Commercial
Cryptographic Algorithms Program (NGCC).
*/
#include "api.h"
#include "drng.h"
#include "parameters.h"
#include "dkecpa.h"
#include "dkecca.h"

extern DRNG_ctx drng_algorithm;

unsigned long long kem_get_pk_len_bytes() { return DKE1_PKBYTES; }
unsigned long long kem_get_sk_len_bytes() { return DKE1_SKBYTES; }
unsigned long long kem_get_ss_len_bytes() { return DKE1_SSBYTES; }
unsigned long long kem_get_ct_len_bytes() { return DKE1_CTBYTES; }

int kem_keygen(unsigned char *pk, unsigned long long *pk_len_bytes,
               unsigned char *sk, unsigned long long *sk_len_bytes) {
    uint8_t coins[DKE1_SEEDBYTES + DKE1_SSBYTES];
    get_random_number(&drng_algorithm, coins, (DKE1_SEEDBYTES + DKE1_SSBYTES) * 8);
    DKE1CCA_keygen_derand(pk, sk, coins);
    return 0;
}

int kem_enc(unsigned char *pk, unsigned long long pk_len_bytes,
            unsigned char *ss, unsigned long long *ss_len_bytes,
            unsigned char *ct, unsigned long long *ct_len_bytes) {
    uint8_t coins[DKE1_SEEDBYTES];
    get_random_number(&drng_algorithm, coins, DKE1_SEEDBYTES * 8);
    get_random_number(&drng_algorithm, coins, DKE1_SEEDBYTES * 8);
    DKE1CCA_enc_derand(ct, ss, pk, coins);
    return 0;
}

int kem_dec(unsigned char *sk, unsigned long long sk_len_bytes,
            unsigned char *ct, unsigned long long ct_len_bytes,
            unsigned char *ss, unsigned long long *ss_len_bytes) {
    DKE1CCA_dec(ss, sk, ct);
    return 0;
}
