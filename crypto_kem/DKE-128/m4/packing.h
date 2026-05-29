#include "parameters.h"
#include "polyvec.h"
#include <stdint.h>


// For keygen -------------------------------------------

// CPA and CCA
void DKE1_packpk(uint8_t bytes[DKE1_PKBYTES],
             const polyvec *pk,
             const uint8_t seed[DKE1_SEEDBYTES]);

// CPA
void DKE1_CPA_packsk(uint8_t r[DKE1_CPA_SKABYTES],
             polyvec *sk);

// For enc ------------------------------------------------

// CPA and CCA
void DKE1_unpackpk(polyvec *pk,
               uint8_t seed[DKE1_SEEDBYTES],
               const uint8_t bytes[DKE1_PKBYTES]);

// CPA
void DKE1_CPA_packciphertext(uint8_t bytes[DKE1_CPA_CTBYTES],
                         polyvec *pb,
                         uint8_t sig[DKE1_SIGNALBYTES]);

// For dec  ------------------------------------------------

// CPA
void DKE1_CPA_unpacksk(polyvec *sk,
               const uint8_t bytes[DKE1_SKBYTES]);


// CPA
void DKE1_CPA_unpackciphertext(polyvec *pb,
                           uint8_t sig[DKE1_SIGNALBYTES],
                           const uint8_t bytes[DKE1_CPA_CTBYTES]);