// #include <stdint.h>
// #include <string.h>
// #include <stdio.h>
// #include <immintrin.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <fcntl.h>
// #include <errno.h>
// #include "param.h"
// #include "kem.h"

// #define NTEST 100000

// #include "mq_ntt.h"
// #include "poly.h"
// #include "time.h"

// int main()
// {
// 	int i;
// 	uint64_t t0, t1, sum;
// 	uint8_t pk[PKLEN], f[SKFLEN], f2[SKF2LEN], c[CTLEN], seed[SEEDLEN], H_pk[SYMBYTES], k[MESSLEN], K[SYMBYTES], m[MESSLEN + PKLEN];

// 	//test correctness
// 	printf("================test correctness================\n\n");
// 	KEM_KeyGen(pk, k, f, f2, H_pk);
// 	KEM_Enc(c, K, pk);
// 	printf("--------DAWN.KEM.Enc Shared Key--------\n");
//     for(int i = 0; i < SYMBYTES; i++)
//     {
//         printf("%d,", K[i]);
//     }
//     printf("\n\n");
// 	KEM_Dec(c, K, pk, k, f, f2, H_pk);
// 	printf("--------DAWN.KEM.Dec Shared Key--------\n");
//     for(int i = 0; i < SYMBYTES; i++)
//     {
//         printf("%d,", K[i]);
//     }
//     printf("\n\n");

// 	//test cpucycles
// 	printf("=================test cpucycles=================\n\n");
// 	sum = 0;
// 	KEM_KeyGen(pk, k, f, f2, H_pk);
// 	for(i = 0; i < NTEST; i++)
// 	{
// 		t0 = __rdtsc();
// 		KEM_KeyGen(pk, k, f, f2, H_pk);
// 		t1 = __rdtsc();
// 		sum += (t1-t0);
// 	}
// 	printf("DAWN.KEM.KeyGen: %llu cycles\n",(sum / NTEST));

// 	KEM_KeyGen(pk, k, f, f2, H_pk);
// 	sum = 0;
// 	for(i = 0; i < NTEST; i++)
// 	{
// 		t0 = __rdtsc();
// 		KEM_Enc(c, K, pk);
// 		t1 = __rdtsc();
// 		sum += (t1-t0);
// 	}
// 	printf("\nDAWN.KEM.Enc:    %llu cycles\n",(sum / NTEST));

// 	KEM_KeyGen(pk, k, f, f2, H_pk);
// 	KEM_Enc(c, K, pk);
// 	sum = 0;
// 	for(i = 0; i < NTEST; i++)
// 	{
// 		t0 = __rdtsc();
// 		KEM_Dec(c, K, pk, k, f, f2, H_pk);
// 		t1 = __rdtsc();
// 		sum += (t1-t0);
// 	}
// 	printf("\nDAWN.KEM.Dec:    %llu cycles\n",(sum / NTEST));
	
//   	return 0;
// }
