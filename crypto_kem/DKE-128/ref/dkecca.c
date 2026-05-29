#include "parameters.h"
#include "dkecpa.h"
#include "auxfunc.h"
#include "dkecca.h"
#include <stdint.h>
#include <string.h>
#include "verify.h"

void DKE1CCA_keygen_derand(uint8_t pk[DKE1_PKBYTES],
                          uint8_t sk[DKE1_SKBYTES],
                          const uint8_t coins[DKE1_SEEDBYTES + DKE1_SSBYTES]) {
    DKE1CPA_keygen_derand(pk, sk, coins);
    memcpy(sk + DKE1_CPA_SKBBYTES, pk, DKE1_PKBYTES); // sk = (skCPA | pk | ...)
    // rejection value rej = coins + DKE1_SEEDBYTES
    memcpy(sk + DKE1_CPA_SKBBYTES + DKE1_PKBYTES, coins + DKE1_SEEDBYTES, DKE1_SSBYTES);
    // sk = (skCPA | pk | rej)

}

void DKE1CCA_enc_derand(uint8_t ct[DKE1_CTBYTES],
                        uint8_t ss[DKE1_SSBYTES],
                        const uint8_t pk[DKE1_PKBYTES],
                        const uint8_t coins[DKE1_SEEDBYTES]) {


    // inits
    uint8_t kr[DKE1_SSBYTES + DKE1_SEEDBYTES + DKE1_N/8];      // will contain (K, r)
    uint8_t buffer[DKE1_SEEDBYTES + DKE1_PKBYTES];             // (coins | pk)
    uint8_t skB[DKE1_SSBYTES];                                 // will contain provisional shared secret



    memcpy(buffer, coins, DKE1_SEEDBYTES);
    memcpy(buffer + DKE1_SEEDBYTES, pk, DKE1_PKBYTES);

    // TODO: I used this auxiliary hash as KDF (provisional).
    pseudoXOF((DKE1_SSBYTES + DKE1_SEEDBYTES)*8 + DKE1_N, buffer, (DKE1_SEEDBYTES + DKE1_PKBYTES)*8 , kr);
    // kr <- (K | r) = HASH(buffer) = HASH(coins | pk)

    // CPA protocol
    DKE1CPA_enc_derand(ct,
                    skB,
                       pk,
                       kr + DKE1_SSBYTES);



    // Compute tag
    /* @WARNING: The previous way of computing the tag comes with a great cost: hasing buffer2 = (ct | skB),
     * where ct consists on relatively many bytes. One possible change is to skip this part
     * and just use skB as mask
     */

    // uint8_t buffer2[DKE1_CPA_CTBYTES + DKE1_SSBYTES]; // will contain (ct | skB)
    // uint8_t tag[DKE1_SSBYTES];                                 // will contain the DKE tag
    // memcpy(buffer2, ct, DKE1_CPA_CTBYTES);
    // memcpy( buffer2 + DKE1_CPA_CTBYTES, skB, DKE1_SSBYTES);
    // pseudoXOF(DKE1_SSBYTES*8, buffer2, (DKE1_CPA_CTBYTES + DKE1_SSBYTES )*8, tag);
    // unsigned int i = 0;
    // for (i  = 0; i < DKE1_SSBYTES; i++) {
    //     tag[i] ^= coins[i];
    // }


    // In this new approach, the tag is saved directly in skB

    // In place one time pad
    unsigned int i = 0;
    for (i  = 0; i < DKE1_SSBYTES; i++) {
        skB[i] ^= coins[i];
    }

    // Emplace the tag
    memcpy(ct + DKE1_CPA_CTBYTES, skB, DKE1_SSBYTES);

    // Extract ss
    memcpy(ss, kr, DKE1_SSBYTES);

}

void DKE1CCA_dec(uint8_t ss[DKE1_SSBYTES],
                 const uint8_t sk[DKE1_SKBYTES],
                 const uint8_t ct[DKE1_CTBYTES]) {

    int fail;                   // 1 if ctA != ctB
    uint8_t r[DKE1_SSBYTES];    // will contain the tag and, after undoing the OTP, the randomness r
    uint8_t skA[DKE1_SSBYTES];
    memcpy(r, ct + DKE1_CPA_CTBYTES, DKE1_SSBYTES); // at this stage, r = tag = ENC coins + mask

    // CPA decryption
    DKE1CPA_dec(skA, sk, ct);

    /*
     * Previous approach:
     */
    // uint8_t buffer2[DKE1_CPA_CTBYTES + DKE1_SSBYTES]; // will contain (ct | skB)
    // uint8_t mask[DKE1_SSBYTES];

    // memcpy(buffer2, ct, DKE1_CPA_CTBYTES);
    // memcpy(buffer2 + DKE1_CPA_CTBYTES , skA, DKE1_SSBYTES);
    // buffer2 = (ctCPA | skA)
    // pseudoXOF(DKE1_SSBYTES*8, buffer2, (DKE1_CPA_CTBYTES + DKE1_SSBYTES)*8, mask);
    // Undo one time pad
    // unsigned int i = 0;
    // for (i  = 0; i < DKE1_SSBYTES; i++) {
    //    r[i] ^= mask[i];
    // }

    // Undo in place one time pad

    unsigned int i = 0;
    for (i  = 0; i < DKE1_SSBYTES; i++) {
        r[i] ^= skA[i];
    }

    // Re-encript
    uint8_t ctA[DKE1_CTBYTES];
    uint8_t ss0[DKE1_SSBYTES];

    // CPA FO encryption
    DKE1CCA_enc_derand(ctA,
                        ss0,
                        sk + DKE1_CPA_SKABYTES,
                        r);

    // ct == ctA?
    fail = DKE1_verify(ct, ctA, DKE1_CPA_CTBYTES);

    // Emplace rejection key in ss
    memcpy(ss, sk + DKE1_SKBYTES - DKE1_SSBYTES, DKE1_SSBYTES);

    // Implicit rejection
    DKE1_cmov(ss, ss0, DKE1_SSBYTES, (uint8_t) (1 - fail));

}



