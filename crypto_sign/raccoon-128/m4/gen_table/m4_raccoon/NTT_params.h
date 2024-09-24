#ifndef NTT_PARAMS_H
#define NTT_PARAMS_H

#define ARRAY_N 512

#define NTT_N 512
#define LOGNTT_N 9

#define Q1 16515073
#define Q2 33292289

// omegaQ1 = Q1pr^((Q1 - 1) / (NTT_N << 1)) mod Q1
#define omegaQ1 12686483
#define omegaQ2 11827776

// invomegaQ1 = omegaQ1^{-1} mod Q1
#define invomegaQ1 1368094
#define invomegaQ2 2304223
// RmodQ1 = 2^32 mod^{+-} Q1
#define RmodQ1 1048316
#define RmodQ2 262015
// invNQ1 = NTT_N^{-1} mod Q1
#define invNQ1 8314945
#define invNQ2 8314945

/*
	(c4q1, c4q2) accounts for FFT^-1 (n) and 4 REDC's (2^-32).
	c4q1 = lift(Mod(q2*n,q1)^-1 * (2^32)^4)
	c4q2 = lift(Mod(q1*n,q2)^-1 * (2^32)^4)
*/
#define MONT_C4Q1 1048477
#define MONT_C4Q2 15632846

#endif

