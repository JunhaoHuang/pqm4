//  xof_sample.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- Samplers and XOF functions

#include <string.h>

#include "racc_param.h"
#include "xof_sample.h"
#include "fips202.h"
#include "mont64.h"

//  Compute mu = H(tr, m) where tr = H(pk), "m" is message of "m_sz" bytes.

void xof_chal_mu(   uint8_t mu[RACC_MU_SZ], const uint8_t tr[RACC_TR_SZ],
                    const uint8_t *m, size_t m_sz)
{
    shake256incctx kec;

    shake256_inc_init(&kec);
    shake256_inc_absorb(&kec, tr, RACC_TR_SZ);
    shake256_inc_absorb(&kec, m, m_sz);
    shake256_inc_finalize(&kec);
    shake256_inc_squeeze(mu, RACC_MU_SZ, &kec);
    shake256_inc_ctx_release(&kec);
}

//  Expand "seed" of "seed_sz" bytes to a uniform polynomial (mod q).
//  The input seed is assumed to alredy contain domain separation.

void xof_sample_q(int64_t r[RACC_N], const uint8_t *seed, size_t seed_sz)
{
    size_t i;
    int64_t x;
    uint8_t buf[8];
    shake256incctx kec;

    //  sample from squeezed output
    shake256_inc_init(&kec);
    shake256_inc_absorb(&kec, seed, seed_sz);
    shake256_inc_finalize(&kec);

    memset(buf, 0, sizeof(buf));
    for (i = 0; i < RACC_N; i++) {
        do {
            shake256_inc_squeeze(buf, (RACC_Q_BITS + 7) / 8, &kec);
            x = get64u_le(buf) & RACC_QMSK;
        } while (x >= RACC_Q);
        r[i] = x;
    }
    shake256_inc_ctx_release(&kec);
}

//  Sample "bits"-wide signed coefficients from "seed[seed_sz]".
//  The input seed is assumed to alredy contain domain separation.

void xof_sample_u(int64_t r[RACC_N], int bits,
                  const uint8_t *seed, size_t seed_sz)
{
    size_t i, blen;
    int64_t x, mask, mid;
    uint8_t buf[8];
    shake256incctx kec;

    blen = (bits + 7) / 8;
    mask = (1ll << bits) - 1;
    mid = 1ll << (bits - 1);

    //  absorb seed
    shake256_inc_init(&kec);
    shake256_inc_absorb(&kec, seed, seed_sz);
    shake256_inc_finalize(&kec);

    //  sample from squeezed outpu
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < RACC_N; i++) {
        shake256_inc_squeeze(buf, blen, &kec);
        x = get64u_le(buf) & mask;
        x ^= mid;  //   two's complement sign bit: 0=pos, 1=neg
        r[i] = mont64_cadd(x - mid, RACC_Q);
    }
}

//  Hash "w" vector with "mu" to produce challenge hash "ch".

void xof_chal_hash( uint8_t ch[RACC_CH_SZ], const uint8_t mu[RACC_MU_SZ],
                    const int64_t w[RACC_K][RACC_N])
{
    size_t i, j;
    uint8_t buf[8];
    shake256incctx kec;

    shake256_inc_init(&kec);

    //  hash of: domain separators 'h', mu, and w in bytes
    buf[0] = 'h';
    buf[1] = RACC_K;
    memset(buf + 2, 0x00, 6);
    shake256_inc_absorb(&kec, buf, 8);

    //  mu
    shake256_inc_absorb(&kec, mu, RACC_MU_SZ);

    //  w: ceil(log2(q)/8) bytes per coefficient
    for (i = 0; i < RACC_K; i++) {
        for (j = 0; j < RACC_N; j++) {
#if ((RACC_LGW + 7) / 8) == 1
            buf[0] = w[i][j];
#else
            put64u_le(buf, w[i][j]);
#endif
            shake256_inc_absorb(&kec, buf, (RACC_LGW + 7) / 8);
        }
    }

    shake256_inc_finalize(&kec);
    shake256_inc_squeeze(ch, RACC_CH_SZ, &kec);
    shake256_inc_ctx_release(&kec);
}

//  Create a challenge polynomial "cp" from a challenge hash "ch".

void xof_chal_poly( int64_t cp[RACC_N], const uint8_t ch[RACC_CH_SZ])
{
    shake256incctx kec;
    uint8_t buf[8];
    size_t i, j;
    int64_t x;

    shake256_inc_init(&kec); //  header
    buf[0] = 'c';
    buf[1] = RACC_W;
    memset(buf + 2, 0x00, 6);
    shake256_inc_absorb(&kec, buf, 8);

    shake256_inc_absorb(&kec, ch, RACC_CH_SZ); //  add ch
    shake256_inc_finalize(&kec);

    for (i = 0; i < RACC_N; i++) {
        cp[i] = 0;
    }

    j = 0;
    while (j < RACC_W) {
        shake256_inc_squeeze(buf, 2, &kec);
        i = get16u_le(buf);
        x = i & 1;
        i = (i >> 1) & (RACC_N - 1);
        if (cp[i] == 0) {
            cp[i] = 2 * x - 1;
            j++;
        }
    }
    shake256_inc_ctx_release(&kec);
}
