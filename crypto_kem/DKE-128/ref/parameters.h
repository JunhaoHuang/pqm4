// SJG

#ifndef PARAMETERS_H
#define PARAMETERS_H


// Protocol parameters--------------------------------------------------------
#define DKE1_N      256     // polynomial's degree
#define DKE1_Q      3329    // modulus
#define DKE1_K      2       // module dimension
#define DKE1_DA     12      // pk rounding parameter
#define DKE1_DB     10      // ct rounding parameter
#define DKE1_L      4       // number of bits for each hint
#define DKE1_NOISE_A      3     // centered binomial parameter for pk
#define DKE1_NOISE_B      3     // centered binomial parameter for ct


// Sizes (bytes)-------------------------------------------------------------
#define DKE1_SEEDBYTES       32

#define DKE1_POLYBYTES       12 * DKE1_N / 8
#define DKE1_POLYVECBYTES    DKE1_K * DKE1_POLYBYTES

#define DKE1_PACOMPRESSEDBYTES        (DKE1_K * DKE1_DA * DKE1_N)/8    // pA'
#define DKE1_PBCOMPRESSEDBYTES        (DKE1_K * DKE1_DB * DKE1_N)/8    // pB'
#define DKE1_SIGNALBYTES     DKE1_L * DKE1_N / 8

#define DKE1_SSBYTES        32    // Bytes in the shared secret key

// Public keys:
#define DKE1_PKBYTES    DKE1_PACOMPRESSEDBYTES + DKE1_SEEDBYTES
#define DKE1_CPA_CTBYTES    DKE1_PBCOMPRESSEDBYTES + DKE1_SIGNALBYTES
#define DKE1_CTBYTES        DKE1_CPA_CTBYTES + DKE1_SSBYTES // ct + tag

// Secret keys:
#define DKE1_CPA_SKABYTES  (DKE1_K * DKE1_N * 12)/8    // just sA
#define DKE1_CPA_SKBBYTES  (DKE1_K * DKE1_N * 12)/8    // just sB
#define DKE1_SKBYTES      DKE1_CPA_SKABYTES + DKE1_PKBYTES + DKE1_SSBYTES
    // sk + pk + rejection value (needed for FO)
// ------------------------------------------------------------------------------

#endif //PARAMETERS_H
