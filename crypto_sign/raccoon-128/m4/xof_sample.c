//  xof_sample.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- Samplers and XOF functions

#include <string.h>

#include "racc_param.h"
#include "xof_sample.h"
#include "fips202.h"
#include "mont64.h"
#include "hal.h"
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

void xof_sample_q_old(int64_t r[RACC_N], const uint8_t *seed, size_t seed_sz)
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

/*************************************************
 * Name:        rej_uniform
 *
 * Description: Sample uniformly random coefficients in [0, Q-1] by
 *              performing rejection sampling on array of random bytes.
 *
 * Arguments:   - int64_t *a: pointer to output array (allocated)
 *              - unsigned int len: number of coefficients to be sampled
 *              - const uint8_t *buf: array of random bytes
 *              - unsigned int buflen: length of array of random bytes
 *
 * Returns number of sampled coefficients. Can be smaller than len if not enough
 * random bytes were given.
 **************************************************/
// static unsigned int rej_uniform_8(int64_t *a,
//                                 unsigned int len,
//                                 const uint8_t *buf,
//                                 unsigned int buflen)
// {
//     unsigned int ctr, pos;
//     int64_t t=0;
//     ctr = pos = 0;
//     while (ctr < len && pos + 49 <= buflen)
//     {
//         uint8_t b0=buf[pos++];//8 bits with each bit as the significant bits in the following 8 6-byte values
//         for(int i=0;i<8;i++)
//         {
//             t=get48u_le(buf+pos);
//             t=(((uint64_t)b0<<48) | t) & RACC_QMSK;
//             b0 >>= 1;
//             pos += 6;

//             if (t < RACC_Q)
//                 a[ctr++] = t;
//             t=0;
//         }
//     }

//     return ctr;
// }

static unsigned int rej_uniform(int64_t *a,
                                unsigned int len,
                                const uint8_t *buf,
                                unsigned int buflen)
{
    unsigned int ctr, pos;
    int64_t t;
    ctr = pos = 0;
    while (ctr < len && pos + 7 <= buflen)
    {
        t=get64u_le(buf+pos) & RACC_QMSK;
        pos += 7;

        if (t < RACC_Q)
            a[ctr++] = t;
    }

    return ctr;
}

#define POLY_UNIFORM_NBLOCKS ((3584+SHAKE256_RATE-1)/SHAKE256_RATE)
void xof_sample_q(int64_t r[RACC_N], const uint8_t *seed, size_t seed_sz)
{
    size_t i, ctr, off;
    uint8_t buf[POLY_UNIFORM_NBLOCKS*SHAKE256_RATE+2];
    unsigned int buflen = POLY_UNIFORM_NBLOCKS*SHAKE256_RATE;
    shake256ctx kec;

    //  sample from squeezed output
    // shake256_inc_init(&kec);
    shake256_absorb(&kec, seed, seed_sz);
    // shake256_inc_finalize(&kec);

    shake256_squeezeblocks(buf, POLY_UNIFORM_NBLOCKS, &kec);
    ctr = rej_uniform(r, RACC_N, buf, buflen);

    // off=0;
    while(ctr<RACC_N){
        off = buflen % 7;
        for (i = 0; i < off; i++)
            buf[i] = buf[buflen - off + i];
        shake256_squeezeblocks(buf + off, 1, &kec);
        buflen=SHAKE256_RATE + off;
        ctr+= rej_uniform(r + ctr, RACC_N - ctr, buf, buflen);
    }

    // shake256_inc_ctx_release(&kec);
}

//  Sample "bits"-wide signed coefficients from "seed[seed_sz]".
//  The input seed is assumed to alredy contain domain separation.


void xof_sample_u_old(int64_t r[RACC_N], int bits,
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

    //  sample from squeezed output
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < RACC_N; i++) {
        shake256_inc_squeeze(buf, blen, &kec);
        x = get64u_le(buf) & mask;
        x ^= mid;  //   two's complement sign bit: 0=pos, 1=neg
        r[i] = mont64_cadd(x - mid, RACC_Q);
    }
}
static unsigned int rej_u(int64_t *r, unsigned int len,
           const uint8_t *buf, unsigned int buflen,
           int bits)
{
    unsigned int ctr, pos;
    int64_t x, mask, mid;
    ctr = pos = 0;
    mask = (1ll << bits) - 1;
    mid = 1ll << (bits - 1);

    while (ctr < len && pos <= buflen)
    {
        x = get64u_le(buf + pos) & mask;
        pos += (bits + 7) / 8;

        x ^= mid; //   two's complement sign bit: 0=pos, 1=neg
        r[ctr++] = mont64_cadd(x - mid, RACC_Q);
    }
    return ctr;
}

#define POLY_U_NBLOCKS(bits, len) \
    (((((bits) + 7) / 8) * (len) + SHAKE256_RATE - 1) / SHAKE256_RATE)

void xof_sample_u(int64_t r[RACC_N], int bits,
                  const uint8_t *seed, size_t seed_sz)
{
    size_t ctr;
    uint8_t buf[POLY_U_NBLOCKS(bits, RACC_N)*SHAKE256_RATE];
    unsigned int buflen = POLY_U_NBLOCKS(bits, RACC_N)*SHAKE256_RATE;
    shake256ctx kec;
    //  absorb seed
    shake256_absorb(&kec, seed, seed_sz);

    shake256_squeezeblocks(buf, POLY_U_NBLOCKS(bits, RACC_N), &kec);

    ctr = rej_u(r, RACC_N, buf, buflen, bits);
    while (ctr < RACC_N) {
        shake256_squeezeblocks(buf, 1, &kec);
        ctr += rej_u(r + ctr, RACC_N - ctr, buf, SHAKE256_RATE, bits);
    }
}

//  Hash "w" vector with "mu" to produce challenge hash "ch".

void xof_chal_hash_old( uint8_t ch[RACC_CH_SZ], const uint8_t mu[RACC_MU_SZ],
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
// #if ((RACC_LGW + 7) / 8) == 1
            buf[0] = w[i][j];
// #else
//             put64u_le(buf, w[i][j]);
// #endif
            shake256_inc_absorb(&kec, buf, (RACC_LGW + 7) / 8);
        }
    }

    shake256_inc_finalize(&kec);
    shake256_inc_squeeze(ch, RACC_CH_SZ, &kec);
    shake256_inc_ctx_release(&kec);
}

void xof_chal_hash(uint8_t ch[RACC_CH_SZ], const uint8_t mu[RACC_MU_SZ],
                   const int64_t w[RACC_K][RACC_N])
{
    size_t i, j;
    uint8_t buf[8+RACC_MU_SZ+RACC_N * RACC_K];
    shake256ctx kec;

    // shake256_inc_init(&kec);

    //  hash of: domain separators 'h', mu, and w in bytes
    buf[0] = 'h';
    buf[1] = RACC_K;
    memset(buf + 2, 0x00, 6);
    memcpy(buf+8, mu, RACC_MU_SZ);

    for(i=0; i<RACC_K;i++){
        for (j = 0; j < RACC_N; j++) {
            buf[8 + RACC_MU_SZ + i * RACC_N + j] = w[i][j];
        }
    }

    //  mu
    shake256_absorb(&kec, buf, 8 + RACC_MU_SZ + RACC_N * RACC_K);

    // shake256_inc_finalize(&kec);
    shake256_squeezeblocks(buf, 1, &kec);
    memcpy(ch, buf, RACC_CH_SZ);
    // shake256_inc_ctx_release(&kec);
}

//  Create a challenge polynomial "cp" from a challenge hash "ch".

void xof_chal_poly_old( int64_t cp[RACC_N], const uint8_t ch[RACC_CH_SZ])
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

void xof_chal_poly(int64_t cp[RACC_N], const uint8_t ch[RACC_CH_SZ])
{
    shake256ctx kec;
    uint8_t buf[SHAKE256_RATE];
    size_t i, j, pos;
    int64_t x;

    buf[0] = 'c';
    buf[1] = RACC_W;
    memset(buf + 2, 0x00, 6);
    memcpy(buf+8, ch, RACC_CH_SZ);

    shake256_absorb(&kec, buf, 8+RACC_CH_SZ); //  add header and ch

    for (i = 0; i < RACC_N; i++)
    {
        cp[i] = 0;
    }
    j = 0; 
    while (j < RACC_W) {
        shake256_squeezeblocks(buf, 1, &kec);
        for (pos = 0; pos + 2 <= SHAKE256_RATE && j < RACC_W; pos += 2) {
            i = get16u_le(buf + pos);
            x = i & 1;
            i = (i >> 1) & (RACC_N - 1);
            if (cp[i] == 0) {
                cp[i] = 2 * x - 1;
                j++;
            }
        }
    }
}