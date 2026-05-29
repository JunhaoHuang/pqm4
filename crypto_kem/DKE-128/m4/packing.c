#include "packing.h"
#include "parameters.h"
#include "polyvec.h"
#include <stdint.h>
#include <string.h>



void DKE1_packpk(uint8_t bytes[DKE1_PKBYTES],
             const polyvec *pk,
             const uint8_t seed[DKE1_SEEDBYTES]) {
    DKE1_polyvec_tobytes(bytes, pk);
    memcpy(bytes + DKE1_PACOMPRESSEDBYTES, seed, DKE1_SEEDBYTES);
}

void DKE1_CPA_packsk(uint8_t bytes[DKE1_CPA_SKABYTES],
                 polyvec *sk) {
    DKE1_polyvec_tobytes(bytes, sk);
}


void DKE1_CPA_packciphertext(uint8_t bytes[DKE1_CPA_CTBYTES],
                         polyvec *pb,
                         uint8_t sig[DKE1_SIGNALBYTES]) {
    DKE1_polyvec_compress10(bytes, pb);
    memcpy(bytes + DKE1_PBCOMPRESSEDBYTES, sig, DKE1_SIGNALBYTES);
}

void DKE1_CPA_unpackciphertext(polyvec *pb,
                           uint8_t sig[DKE1_SIGNALBYTES],
                           const uint8_t bytes[DKE1_CPA_CTBYTES]) {
    DKE1_polyvec_decompress10(pb, bytes);
    memcpy(sig, bytes + DKE1_PBCOMPRESSEDBYTES, DKE1_SIGNALBYTES);
}


