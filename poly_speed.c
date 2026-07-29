#include "api.h"
#include "hal.h"
#include "sendfn.h"
#include "racc_core.h"
#include "mask_random.h"
#include "racc_param.h"
#include "param_list.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "polyr.h"
#include "mont32.h"
#include "xof_sample.h"
#include "fips202.h"
#include "racc_serial.h"
#define MLEN 59

// https://stackoverflow.com/a/1489985/1711232
#define PASTER(x, y) x##y
#define EVALUATOR(x, y) PASTER(x, y)
#define NAMESPACE(fun) EVALUATOR(MUPQ_NAMESPACE, fun)

// use different names so we can have empty namespaces
#define MUPQ_CRYPTO_PUBLICKEYBYTES NAMESPACE(CRYPTO_PUBLICKEYBYTES)
#define MUPQ_CRYPTO_SECRETKEYBYTES NAMESPACE(CRYPTO_SECRETKEYBYTES)
#define MUPQ_CRYPTO_BYTES NAMESPACE(CRYPTO_BYTES)
#define MUPQ_CRYPTO_ALGNAME NAMESPACE(CRYPTO_ALGNAME)

#define MUPQ_crypto_sign_keypair NAMESPACE(crypto_sign_keypair)
#define MUPQ_crypto_sign NAMESPACE(crypto_sign)
#define MUPQ_crypto_sign_open NAMESPACE(crypto_sign_open)
#define MUPQ_crypto_sign_signature NAMESPACE(crypto_sign_signature)
#define MUPQ_crypto_sign_verify NAMESPACE(crypto_sign_verify)

#define printcycles(S, U) send_unsignedll((S), (U))

typedef uint32_t uint32;
static uint32 seed[32] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3, 8, 3, 2, 7, 9, 5};
static uint32 in[12];
static uint32 out[8];
static int outleft = 0;

#define ROTATE(x, b) (((x) << (b)) | ((x) >> (32 - (b))))
#define MUSH(i, b) x = t[i] += (((x ^ seed[i]) + sum) ^ ROTATE(x, b));

static void surf(void)
{
	uint32 t[12];
	uint32 x;
	uint32 sum = 0;
	int r;
	int i;
	int loop;

	for (i = 0; i < 12; ++i)
		t[i] = in[i] ^ seed[12 + i];
	for (i = 0; i < 8; ++i)
		out[i] = seed[24 + i];
	x = t[11];
	for (loop = 0; loop < 2; ++loop)
	{
		for (r = 0; r < 16; ++r)
		{
			sum += 0x9e3779b9;
			MUSH(0, 5)
			MUSH(1, 7)
			MUSH(2, 9)
			MUSH(3, 13)
				MUSH(4, 5) MUSH(5, 7) MUSH(6, 9) MUSH(7, 13)
					MUSH(8, 5) MUSH(9, 7) MUSH(10, 9) MUSH(11, 13)
		}
		for (i = 0; i < 8; ++i)
			out[i] ^= t[i + 4];
	}
}

int randombytes(uint8_t *x, size_t xlen)
{
	while (xlen > 0)
	{
		if (!outleft)
		{
			if (!++in[0])
				if (!++in[1])
					if (!++in[2])
						++in[3];
			surf();
			outleft = 8;
		}
		*x = out[--outleft];
		++x;
		--xlen;
	}
	return 0;
}
#define NUM_BYTES 1280
void test_shake256()
{
	int block = (NUM_BYTES + SHAKE256_RATE - 1) / SHAKE256_RATE;
	unsigned long long t0, t1;
	shake256ctx ctx;
	shake256incctx incctx;
	int incsize;

	uint8_t input[block * SHAKE256_RATE];
	randombytes(input, sizeof(input));

	t0 = hal_get_time();
	shake256_absorb(&ctx, input, sizeof(input));
	t1 = hal_get_time();
	printcycles("SHAKE256 Absorb cycles:", t1 - t0);

	t0 = hal_get_time();
	shake256_squeezeblocks(input, block, &ctx);
	t1 = hal_get_time();
	printcycles("SHAKE256 Squeeze cycles:", t1 - t0);

	for (incsize = 1; incsize <= SHAKE256_RATE; incsize++)
	{
		shake256_inc_init(&incctx);

		t0 = hal_get_time();
		for (int i = 0; i < block * SHAKE256_RATE; i += incsize)
		{
			int sz = incsize;
			if (i + sz > block * SHAKE256_RATE)
				sz = block * SHAKE256_RATE - i;
			shake256_inc_absorb(&incctx, input + i, sz);
		}
		t1 = hal_get_time();
		printcycles("Incremental size: ", incsize);
		printcycles("SHAKE256 Inc Absorb cycles:", t1 - t0);

		t0 = hal_get_time();
		for (int i = 0; i < block * SHAKE256_RATE; i += incsize)
		{
			int sz = incsize;
			if (i + sz > block * SHAKE256_RATE)
				sz = block * SHAKE256_RATE - i;
			shake256_inc_squeeze(input, sz, &incctx);
		}

		t1 = hal_get_time();
		printcycles("SHAKE256 Inc Squeeze cycles:", t1 - t0);

		shake256_inc_ctx_release(&incctx);
	}
}

void timing_poly_operations()
{
	int64_t a[RACC_N], b[RACC_N], d[RACC_K][RACC_N];
	uint8_t sseed[21] = "Raccoon128SampleQ", mu[RACC_MU_SZ] = "Raccoon128SampleQ", ch[RACC_CH_SZ];

	unsigned long long t0, t1;
	int i;
	// hal_setup(CLOCK_BENCHMARK);

	// hal_send_str("==========================");

	for (i = 0; i < MUPQ_ITERATIONS; i++)
	{
		// 32-bit NTT
		t0 = hal_get_time();
		polyr_fntt(a);
		t1 = hal_get_time();
		printcycles("polyr_fntt cycles:", t1 - t0);

		// polynomial sampling
		t0 = hal_get_time();
		xof_sample_q(a, sseed, sizeof(sseed));
		t1 = hal_get_time();
		printcycles("xof_sample_q cycles:", t1 - t0);

		t0 = hal_get_time();
		xof_sample_u(a, RACC_UT, sseed, sizeof(sseed));
		t1 = hal_get_time();
		printcycles("xof_sample_ut cycles:", t1 - t0);

		t0 = hal_get_time();
		xof_sample_u(a, RACC_UW, sseed, sizeof(sseed));
		t1 = hal_get_time();
		printcycles("xof_sample_uw cycles:", t1 - t0);

		t0 = hal_get_time();
		xof_chal_hash(ch, mu, d);
		t1 = hal_get_time();
		printcycles("xof_chal_hash cycles:", t1 - t0);

		t0 = hal_get_time();
		xof_chal_poly(a, ch);
		t1 = hal_get_time();
		printcycles("xof_chal_poly cycles:", t1 - t0);

		// polynomial arithmetic
		t0 = hal_get_time();
		polyr_add(a, a, b);
		t1 = hal_get_time();
		printcycles("polyr_add cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_sub(a, a, b);
		t1 = hal_get_time();
		printcycles("polyr_sub cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_addq(a, a, b);
		t1 = hal_get_time();
		printcycles("polyr_addq cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_subq(a, a, b);
		t1 = hal_get_time();
		printcycles("polyr_subq cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_addm(a, a, b, RACC_Q);
		t1 = hal_get_time();
		printcycles("polyr_addm cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_subm(a, a, b, RACC_Q);
		t1 = hal_get_time();
		printcycles("polyr_subm cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_shlm(a, b, RACC_NUT, RACC_Q);
		t1 = hal_get_time();
		printcycles("polyr_shlm cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_shrm(a, b, RACC_NUT, RACC_Q);
		t1 = hal_get_time();
		printcycles("polyr_shrm cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_fntt(a);
		t1 = hal_get_time();
		printcycles("polyr_fntt cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr_intt(a);
		t1 = hal_get_time();
		printcycles("polyr_intt cycles:", t1 - t0);

		t0 = hal_get_time();
		polyr2_split(a);
		t1 = hal_get_time();
		printcycles("polyr2_split cycles:", t1 - t0);
#ifdef RACCOON_M4
		t0 = hal_get_time();
		polyr2_split_neg(a);
		t1 = hal_get_time();
		printcycles("polyr2_split_neg cycles:", t1 - t0);
#endif
		t0 = hal_get_time();
		polyr2_join(a, MONT_D2Q1, MONT_D2Q2);
		t1 = hal_get_time();
		printcycles("polyr2_join cycles:", t1 - t0);

		hal_send_str("+");
	}
}
int main(void)
{
	int64_t c[RACC_D][RACC_N], a[RACC_N];
	int64_t ai[RACC_ELL][RACC_N];
	uint8_t sseed[RACC_AS_SZ] = "Raccoon128";
	racc_sk_t sk;
	uint8_t b[CRYPTO_SECRETKEYBYTES];
	mask_random_t mrg;
	//  intialize the mask random generator
	mask_random_init(&mrg);

	unsigned long long t0, t1;
	int i, j, k;
	hal_setup(CLOCK_BENCHMARK);

	hal_send_str("==========================");

	timing_poly_operations();
	test_shake256();
	for (i = 0; i < MUPQ_ITERATIONS; i++)
	{
		// Masking gadgets and matrix operations
		t0 = hal_get_time();
		for (k = 0; k < RACC_K; k++)
		{
			//  --- 2.  A := ExpandA(seed)
			for (j = 0; j < RACC_ELL; j++)
			{
				expand_aij(ai[j], k, j, sseed);
			}
		}
		t1 = hal_get_time();
		printcycles("gen_matrix cycles:", t1 - t0);

		t0 = hal_get_time();
		racc_encode_sk(b, &sk);
		t1 = hal_get_time();
		printcycles("racc_encode_sk cycles:", t1 - t0);

		t0 = hal_get_time();
		racc_decode_sk(&sk, b);
		t1 = hal_get_time();
		printcycles("racc_decode_sk cycles:", t1 - t0);

		t0 = hal_get_time();
		zero_encoding(c, &mrg);
		t1 = hal_get_time();
		printcycles("zero_encoding cycles:", t1 - t0);

		t0 = hal_get_time();
		racc_refresh(c, &mrg);
		t1 = hal_get_time();
		printcycles("racc_refresh cycles:", t1 - t0);

#if MEM_OPT != 2
		t0 = hal_get_time();
		racc_ntt_refresh(c, &mrg);
		t1 = hal_get_time();
		printcycles("racc_ntt_refresh cycles:", t1 - t0);
#else
		t0 = hal_get_time();
		racc_ntt_refresh_neg(c, &mrg);
		t1 = hal_get_time();
		printcycles("racc_ntt_refresh_neg cycles:", t1 - t0);
#endif
		t0 = hal_get_time();
		racc_decode(a, c);
		t1 = hal_get_time();
		printcycles("racc_decode cycles:", t1 - t0);

		t0 = hal_get_time();
		add_rep_noise(c, 0, RACC_UW, &mrg);
		t1 = hal_get_time();
		printcycles("add_rep_noise cycles:", t1 - t0);

		hal_send_str("+");
	}
	hal_send_str("#");
	return 0;
}
