//  racc_core.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- core scheme.
#include "api.h"
#include <string.h>
#include "plat_local.h"
#include "racc_core.h"
#include "polyr.h"
#include "mont32.h"
#include "mont64.h"
#include "ct_util.h"
#include "xof_sample.h"
#include "randombytes.h"
#include "mask_random.h"
#include "racc_serial.h"

#ifdef XDEBUG
#include <stdio.h>
#include "sendfn.h"
#include "hal.h"
static void printbytes(const unsigned char *x, unsigned long long xlen)
{
    char outs[2 * xlen + 1];
    unsigned long long i;
    for (i = 0; i < xlen; i++)
        sprintf(outs + 2 * i, "%02X", x[i]);
    outs[2 * xlen] = 0;
    hal_send_str(outs);
}
static void print_array_64(uint64_t *x, unsigned long long xlen)
{
    char outs[9 * xlen + 1];
    for (unsigned long long i = 0; i < xlen; i++)
        sprintf(outs + 8 * i, "%016llX,", x[i]);
    outs[8 * xlen] = 0;
    hal_send_str(outs);
}

#define MY_ASSERT(xi, expr, msg)              \
    if (!(expr))                              \
    {                                         \
        hal_send_str(msg);                    \
        send_unsignedll("error number:", xi); \
        break;                                \
    }
static void check_overflow(int64_t *x, unsigned long long xlen, int64_t m, char *msg)
{
    hal_send_str("checking bounds!");
    for (unsigned long long i = 0; i < xlen; i++)
    {
        MY_ASSERT(x[i], x[i] >= -m && x[i] <= m, msg);
        (void)x[i];
    }
}
#endif

//  ExpandA(): Use domain separated XOF to create matrix elements

static void expand_aij(int64_t aij[RACC_N], int i_k, int i_ell,
                       const uint8_t seed[RACC_AS_SZ])
{
    uint8_t buf[RACC_AS_SZ + 8];

    //  --- 3.  hdrA := Ser8(65, i, j, 0, 0, 0, 0, 0)
    buf[0] = 'A'; //  ascii 65
    buf[1] = i_k;
    buf[2] = i_ell;
    memset(buf + 3, 0x00, 8 - 3);

    //  --- 4.  Ai,j <- SampleQ(hdrA, seed)
    memcpy(buf + 8, seed, RACC_AS_SZ);
    xof_sample_q(aij, buf, RACC_AS_SZ + 8);

    //  converted to NTT domain
    polyr_fntt(aij);
}

//  Decode(): Collapse shares
// ||mi||*d
void racc_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N])
{
#if RACC_D == 1
    polyr_copy(r, m[0]);
#else
    int i;
    polyr_add(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++)
    {
        polyr_add(r, r, m[i]);
    }
#endif
}

//  Decode(): Collapse shares (possibly split CRT arithmetic)
// ||r||*d
void racc_ntt_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N])
{
#if RACC_D == 1
    polyr_copy(r, m[0]);
#elif RACC_D == 32
    int i;
    polyr2_add(r, m[0], m[1]);
    for (i = 2; i < 21; i++)
    {
        polyr2_add(r, r, m[i]);
    }
    polyr2_reduce_q2(r, r);
    for (i = 21; i < RACC_D; i++)
    {
        polyr2_add(r, r, m[i]);
    }
#else
    int i;
    polyr2_add(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++)
    {
        polyr2_add(r, r, m[i]);
    }
#endif
}

//  ZeroEncoding(d) -> [[z]]d
//  in-place version
// coefficient grows by log(d)*q; maximum: 5q for d=32
void zero_encoding(int64_t z[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void)mrg;
    polyr_zero(z[0]);
#else
    int i, j, d;
    int64_t r[RACC_N];

    //  d = 2
    for (i = 0; i < RACC_D; i += 2)
    {
        mask_random_poly(mrg, z[i], i);
        polyr_neg(z[i + 1], z[i]);
    }

    //  d = 4, 8, ..
    d = 2;
    while (d < RACC_D)
    {
        for (i = 0; i < RACC_D; i += 2 * d)
        {
            for (j = i; j < i + d; j++)
            {
                mask_random_poly(mrg, r, j);
                polyr_add(z[j], z[j], r);
                polyr_sub(z[j + d], z[j + d], r);
            }
        }
        d <<= 1;
    }
#endif
}

//  Refresh([[x]]) -> [[x]]′
// coefficient grows by ||x||+log(d)*q;
void racc_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void)x;
    (void)mrg;
#else
    int i;
    int64_t z[RACC_D][RACC_N];

    //  --- 1.  [[z]] <- ZeroEncoding(d)
    zero_encoding(z, mrg);

    //  --- 2.  return [[x]]' := [[x]] + [[z]]
    for (i = 0; i < RACC_D; i++)
    {
        polyr_add(x[i], x[i], z[i]);
    }
#endif
}

//  Refresh([[x]]) -> [[x]]′ ( NTT domain )
#if MEM_OPT != 2
// x is negative; remain the same sign -x-z
void racc_ntt_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void)x;
    (void)mrg;
#else
    int i;
    int64_t z[RACC_D][RACC_N];

    //  --- 1.  [[z]] <- ZeroEncoding(d)
    zero_encoding(z, mrg);

    //  --- 2.  return [[x]]' := [[x]] + [[z]]
    for (i = 0; i < RACC_D; i++)
    {
        polyr2_split_neg(z[i]);
        polyr2_add(x[i], x[i], z[i]);
    }
#endif
}
#endif

// x is negative; transform x to its opposite sign: z-(-x)=z+x
// ||x||+q_i
void racc_ntt_refresh_neg(int64_t x[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    polyr2_neg(x[0], x[0]);
    (void)mrg;
#else
    int i;
    int64_t z[RACC_D][RACC_N];

    //  --- 1.  [[z]] <- ZeroEncoding(d)
    zero_encoding(z, mrg);

    //  --- 2.  return [[x]]' := [[x]] + [[z]]
    for (i = 0; i < RACC_D; i++)
    {
        polyr2_split(z[i]);
        polyr2_sub(x[i], z[i], x[i]);
    }
#endif
}

#if MEM_OPT == 2
// use the noise in precomputed buffer
// coefficient grows by ||vi||+rep*RACC_U{W,T}+rep*log(d)*q;
void add_rep_noise_buf(int64_t vi[RACC_D][RACC_N],
                       int i_v, int u, uint8_t *in, mask_random_t *mrg)
{
    int i_rep, j;
    uint8_t buf[RACC_SEC + 8];
    int64_t r[RACC_N];

    //  --- 1.  for i in [len(v)] do                        [caller]

    //  --- 2.  for i_rep in [rep] do
    for (i_rep = 0; i_rep < RACC_REP; i_rep++)
    {

        //  --- 3.  for j in [d] do:
        for (j = 0; j < RACC_D; j++)
        {

            //  --- 4.  sigma <- {0,1}^kappa
            memcpy(buf + 8, in + j * RACC_SEC + i_rep * RACC_D * RACC_SEC, RACC_SEC);

            //  --- 5.  hdr_u := Ser8('u' || i_rep || i_v || j || (0) || seed)
            buf[0] = 'u'; //  ascii 117
            buf[1] = i_rep;
            buf[2] = i_v;
            buf[3] = j;
            memset(buf + 4, 0x00, 8 - 4);

            //  --- 6.  v_ij <- v_ij + SampleU(hdr_u, sigma, u)
            xof_sample_u(r, u, buf, RACC_SEC + 8);
            polyr_add(vi[j], vi[j], r);
        }
        // hal_send_str("racc_refresh begin");
        //  --- [[v_i]] <- Refresh([[v_i]])
        racc_refresh(vi, mrg);
    }
}

#endif
//  AddRepNoise([[v]], u, rep) -> [[v]]
//  Add repeated noise to a polynomial (vector at index i_v)
// coefficient grows by ||vi||+rep*RACC_U{W,T}+rep*log(d)*q;
void add_rep_noise(int64_t vi[RACC_D][RACC_N],
                   int i_v, int u, mask_random_t *mrg)
{
    int i_rep, j;
    uint8_t buf[RACC_SEC + 8];
    int64_t r[RACC_N];

    //  --- 1.  for i in [len(v)] do                        [caller]

    //  --- 2.  for i_rep in [rep] do
    for (i_rep = 0; i_rep < RACC_REP; i_rep++)
    {

        //  --- 3.  for j in [d] do:
        for (j = 0; j < RACC_D; j++)
        {

            //  --- 4.  sigma <- {0,1}^kappa
            randombytes(buf + 8, RACC_SEC);

            //  --- 5.  hdr_u := Ser8('u' || i_rep || i_v || j || (0) || seed)
            buf[0] = 'u'; //  ascii 117
            buf[1] = i_rep;
            buf[2] = i_v;
            buf[3] = j;
            memset(buf + 4, 0x00, 8 - 4);

            //  --- 6.  v_ij <- v_ij + SampleU(hdr_u, sigma, u)
            xof_sample_u(r, u, buf, RACC_SEC + 8);
            polyr_add(vi[j], vi[j], r);
        }

        //  --- [[v_i]] <- Refresh([[v_i]])
        racc_refresh(vi, mrg);
    }
}

//  "rounding" shift right
extern void polyr_shrm42_asm(int64_t *r, int32_t q);
extern void polyr_shrm44_asm(int64_t *r, int32_t q);
// static void round_shift_r(int64_t *r, int64_t q, int s)
// {
//     if(s==RACC_NUT){
//         polyr_shrm42_asm(r,(int32_t)q);
//     }
//     else{
//         polyr_shrm44_asm(r,(int32_t)q);
//     }
// }

//  CheckBounds(sig) -> {OK or FAIL}

static bool racc_check_bounds(const int64_t h[RACC_K][RACC_N],
                              const int64_t z[RACC_ELL][RACC_N])
{
    int i, j;
    int64_t x, h22, hoo, z22, zoo;

    //  --- 1.  if |sig| != |sig|default return FAIL        [caller]
    //  --- 2.  (c hash, h, z) := sig                       [caller]

    //  Infinity and L2 norms for hint
    h22 = 0;
    hoo = 0;
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = h[i][j];
            if (x < 0) //  x mod q  (non-negative)
                x = -x;
            if (x > hoo)
                hoo = x;
            h22 += (x * x);
        }
    }

    //  Infinity norm and scaled L2 norm for z
    z22 = 0;
    zoo = 0;
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = z[i][j];
            if (x < 0) //  x mod q  (non-negative)
                x += RACC_Q;
            if (x > (RACC_Q / 2)) //  absolute value
                x = RACC_Q - x;
            if (x > zoo)
                zoo = x;

            //  --- 6.  z2 := sum_i [ abs(zi) / 2^32 ]^2
            x >>= 32; //  scale to avoid overflow
            z22 += (x * x);
        }
    }

    //  --- 3:  if ||h||oo > round(Boo/2^nuw) return FAIL
    if (hoo > ((int64_t)(RACC_BOO + ((int64_t)1l << (RACC_NUW - 1))) >> RACC_NUW))
        return false;

    //  --- 4.  if ||z||oo > Boo return FAIL
    if (zoo > RACC_BOO)
        return false;

    //  --- 5.  h2 := 2^(2*nuw - 64) * ||h||^2
    //  --- 7.  if (h2 + z2) > 2^-64*B22 return FAIL
    if (((h22 << (2 * RACC_NUW - 64)) + z22) > RACC_B22)
        return false;

    //  --- 8.  return OK
    return true;
}

#if MEM_OPT == 2
static void racc_check_bounds_h(int64_t *h22, int64_t *hoo, const int64_t h[RACC_N])
{
    int j;
    int64_t x;

    //  --- 1.  if |sig| != |sig|default return FAIL        [caller]
    //  --- 2.  (c hash, h, z) := sig                       [caller]

    //  Infinity and L2 norms for hint
    for (j = 0; j < RACC_N; j++)
    {
        x = h[j];
        if (x < 0) //  x mod q  (non-negative)
            x = -x;
        if (x > (*hoo))
            (*hoo) = x;
        (*h22) += (x * x);
    }
}

static void racc_check_bounds_z(int64_t *z22, int64_t *zoo, const int64_t z[RACC_N])
{
    int j;
    int64_t x;

    //  --- 1.  if |sig| != |sig|default return FAIL        [caller]
    //  --- 2.  (c hash, h, z) := sig                       [caller]

    //  Infinity norm and scaled L2 norm for z
    for (j = 0; j < RACC_N; j++)
    {
        x = z[j];
        if (x < 0) //  x mod q  (non-negative)
            x += RACC_Q;
        if (x > (RACC_Q / 2)) //  absolute value
            x = RACC_Q - x;
        if (x > (*zoo))
            (*zoo) = x;

        //  --- 6.  z2 := sum_i [ abs(zi) / 2^32 ]^2
        x >>= 32; //  scale to avoid overflow
        (*z22) += (x * x);
    }
}

static bool racc_check_bounds_zh(int64_t z22, int64_t zoo, int64_t h22, int64_t hoo)
{
    //  --- 3:  if ||h||oo > round(Boo/2^nuw) return FAIL
    if (hoo > ((int64_t)(RACC_BOO + ((int64_t)1l << (RACC_NUW - 1))) >> RACC_NUW))
    {
        return false;
    }

    //  --- 4.  if ||z||oo > Boo return FAIL
    if (zoo > RACC_BOO)
    {
        return false;
    }

    //  --- 5.  h2 := 2^(2*nuw - 64) * ||h||^2
    //  --- 7.  if (h2 + z2) > 2^-64*B22 return FAIL
    if (((h22 << (2 * RACC_NUW - 64)) + z22) > RACC_B22)
    {
        return false;
    }

    //  --- 8.  return OK
    return true;
}
#endif

#if MEM_OPT > 0
int racc_core_keygen(unsigned char *pk, racc_sk_t *sk)
{
    int i, j, k;
    size_t l_pk = 0;
    int64_t(*ai)[RACC_N];
    int64_t mt[RACC_D][RACC_N];
    int64_t *t;
    t = mt[0];
    ai = sk->pk.t; // reuse sk->pk.t

    mask_random_t mrg;
    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  seed <- {0,1}^kappa
    randombytes(pk, RACC_AS_SZ);
    //  encode A seed
    l_pk += RACC_AS_SZ;

    for (i = 0; i < RACC_ELL; i++)
    {
        //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
        zero_encoding(sk->s[i], &mrg);

        //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
        add_rep_noise(sk->s[i], i, RACC_UT, &mrg); // can accept redundancy

        for (j = 0; j < RACC_D; j++)
        {
            polyr_fntt(sk->s[i][j]);
        }
    }

    for (i = 0; i < RACC_K; i++)
    {
        //  --- 2.  A := ExpandA(seed)
        for (j = 0; j < RACC_ELL; j++)
        {
            expand_aij(ai[j], i, j, pk);
        }

        //  --- 5.  [[t]] := A * [[s]]
        for (j = 0; j < RACC_D; j++)
        {
            polyr_ntt_cmul(mt[j], sk->s[0][j], ai[0]);
            for (k = 1; k < RACC_ELL; k++)
            {
                polyr_ntt_mula(mt[j], sk->s[k][j], ai[k], mt[j]);
            }
            polyr_intt(mt[j]);
        }

        //  --- 6.  [[t]] <- AddRepNoise([[t]], ut, rep)
        add_rep_noise(mt, i, RACC_UT, &mrg);

        //  --- 7.  t := Decode([[t]])
        racc_decode(t, mt);
        polyr_reduce(t, t);
        //  --- 8.  t := round( t_m )_q->q_t
        polyr_shrm42_asm(t, RACC_QT); // 49-42=7-bit

        // --- encode pk on-the-fly
        l_pk = racc_encode_pk_k(pk, t, l_pk);
    }

    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    return (l_pk != CRYPTO_PUBLICKEYBYTES);
}
#else

//  === racc_core_keygen ===
//  Generate a public-secret keypair ("pk", "sk").
void racc_core_keygen(racc_pk_t *pk, racc_sk_t *sk)
{
    int i, j, k;
    int64_t ai[RACC_ELL][RACC_N];
    int64_t mt[RACC_D][RACC_N];
    mask_random_t mrg;

    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  seed <- {0,1}^kappa
    randombytes(pk->a_seed, RACC_AS_SZ);

    for (i = 0; i < RACC_ELL; i++)
    {

        //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
        zero_encoding(sk->s[i], &mrg);

        //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
        add_rep_noise(sk->s[i], i, RACC_UT, &mrg);

        for (j = 0; j < RACC_D; j++)
        {
            polyr_fntt(sk->s[i][j]);
        }
    }

    for (i = 0; i < RACC_K; i++)
    {

        //  --- 2.  A := ExpandA(seed)
        for (j = 0; j < RACC_ELL; j++)
        {
            expand_aij(ai[j], i, j, pk->a_seed);
        }

        //  --- 5.  [[t]] := A * [[s]]
        for (j = 0; j < RACC_D; j++)
        {
            polyr_ntt_cmul(mt[j], sk->s[0][j], ai[0]);
            for (k = 1; k < RACC_ELL; k++)
            {
                polyr_ntt_mula(mt[j], sk->s[k][j], ai[k], mt[j]);
            }
            polyr_intt(mt[j]);
        }
        //  --- 6.  [[t]] <- AddRepNoise([[t]], ut, rep)
        add_rep_noise(mt, i, RACC_UT, &mrg);

        //  --- 7.  t := Decode([[t]])
        racc_decode(pk->t[i], mt);
        polyr_reduce(pk->t[i], pk->t[i]);

        //  --- 8.  t := round( t_m )_q->q_t
        polyr_shrm42_asm(pk->t[i], RACC_QT); // 49-42=7-bit
    }

    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    memcpy(&sk->pk, pk, sizeof(racc_pk_t));
}
#endif

//  === racc_core_sign ===
//  Create a detached signature "sig" for digest "mu" using secret key "sk".
#if MEM_OPT == 2
int racc_core_sign(uint8_t *sig, const uint8_t mu[RACC_MU_SZ],
                   racc_sk_t *sk)
{
    int i, j, k;
    int64_t *aij;
    int64_t mr[RACC_D][RACC_N];
    int64_t mw[RACC_D][RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t u[RACC_N], c_poly[RACC_N], y[RACC_N], *h, *z;
    uint8_t seed_buf[RACC_AS_SZ + RACC_ELL * RACC_D * RACC_REP * RACC_SEC + RACC_K * RACC_D * RACC_REP * RACC_SEC];
    uint8_t *seed, *seed_mr, *seed_mw;
    size_t l_sig, pre_k;
    uint8_t pre_z;
    int64_t h22, hoo, z22, zoo;

    aij = u; // reuse u for aij
    z = u;   // reuse u for z
    h = u;   // reuse u for h

    seed = seed_buf;
    seed_mr = seed + RACC_AS_SZ;
    seed_mw = seed_mr + RACC_ELL * RACC_REP * RACC_D * RACC_SEC;

    bool rsp = false;
    mask_random_t mrg;
    //  --- 1.  (vk, [[s]]) := [[sk]], (seed, t) := vk      [ caller ]
    //  --- 2.  mu := H( H(vk) || msg )                     [ caller ]

    // --- get pk.a_seed
    memcpy(seed, sk->pk.a_seed, RACC_AS_SZ);

    do // move the racc_api do-while here.
    {
        //  intialize the mask random generator and index
        mask_random_init(&mrg);
        do
        {
            h22 = 0;
            z22 = 0;
            hoo = 0;
            zoo = 0;
            l_sig = 0;

            //  --- 1.  random noise for mr, mw.
            randombytes(seed_buf + RACC_AS_SZ, sizeof(seed_buf) - RACC_AS_SZ);

            for (i = 0; i < RACC_K; i++)
            {
                //  --- 3.  A := ExpandA(seed)
                //  --- 6.  [[w]] := A * [[r]]
                expand_aij(aij, i, 0, sk->pk.a_seed);
                //  --- 4.  [[r]] <- ZeroEncoding()
                zero_encoding(mr, &mrg);
                //  --- 5.  [[r]] <- AddRepNoise([[r]], uw, rep)
                add_rep_noise_buf(mr, 0, RACC_UW, seed_mr + 0 * RACC_REP * RACC_D * RACC_SEC, &mrg);

                for (j = 0; j < RACC_D; j++)
                {
                    polyr_fntt(mr[j]);
                    polyr_ntt_cmul(mw[j], mr[j], aij);
                }
                for (k = 1; k < RACC_ELL; k++)
                {
                    expand_aij(aij, i, k, sk->pk.a_seed);
                    zero_encoding(mr, &mrg);
                    add_rep_noise_buf(mr, k, RACC_UW, seed_mr + k * RACC_REP * RACC_D * RACC_SEC, &mrg);

                    for (j = 0; j < RACC_D; j++)
                    {
                        polyr_fntt(mr[j]);
                        polyr_ntt_mula(mw[j], mr[j], aij, mw[j]);
                    }
                }
                for (j = 0; j < RACC_D; j++)
                {
                    polyr_intt(mw[j]);
                }

                //  --- 7.  [[w]] <- AddRepNoise([[w]], uw, rep)
                add_rep_noise_buf(mw, i, RACC_UW, seed_mw + i * RACC_REP * RACC_D * RACC_SEC, &mrg);

                //  --- 8.  w := Decode([[w]])
                racc_decode(vw[i], mw);
                polyr_reduce(vw[i], vw[i]);
                //  --- 9.  w := round( w )_q->q_w
                polyr_shrm44_asm(vw[i], RACC_QW);
            }

            //  --- 10. c_hash := ChalHash(w, mu)
            xof_chal_hash(sig, mu, vw);

            l_sig = RACC_CH_SZ;

            //  --- 11. c_poly := ChalPoly(c_hash)
            xof_chal_poly(c_poly, sig);
            polyr_fntt(c_poly);

            pre_k = 0;
            pre_z = 0;
            for (i = 0; i < RACC_ELL; i++)
            {
                //  --- 12. [[s]] <- Refresh([[s]])
                racc_ntt_refresh_neg(sk->s[i], &mrg);

                //  --- 13. [[r]] <- Refresh([[r]])
                zero_encoding(mr, &mrg);
                add_rep_noise_buf(mr, i, RACC_UW, seed_mr + i * RACC_REP * RACC_D * RACC_SEC, &mrg);

                //  --- 14. [[z]] := c_poly * [[s]] + [[r]]
                for (j = 0; j < RACC_D; j++)
                {
                    polyr_fntt(mr[j]);
                    //  due to 2x Montgomery
                    polyr_ntt_smul(u, mr[j], MONT_RI1, MONT_RI2);
                    polyr_ntt_mula(mr[j], c_poly, sk->s[i][j], u);
                }

                //  --- 15. [[r]] <- Refresh([[r]])
                racc_ntt_refresh_neg(mr, &mrg);

                //  --- 16. z := Decode([[z]])
                racc_ntt_decode(z, mr);

                //  Two consecutive multiplications: Montgomery adjustment
                polyr_ntt_smul(vz[i], z, -MONT_RRR1, -MONT_RRR2); // negative & normal domain
#if RACC_D > 2
                polyr2_reduce(z, z);
#endif
                //  Decode for signature
                polyr_intt(z);
                polyr_reduce(z, z);
                // check bounds and decode on-the-fly
                racc_check_bounds_z(&z22, &zoo, z);
                l_sig = racc_encode_sig_z(sig, CRYPTO_BYTES, l_sig, &pre_z, &pre_k, z);
            }

            for (i = 0; i < RACC_K; i++)
            {
                //  --- 17. y := A*z - 2^{nu_t} * c_poly * t
                expand_aij(aij, i, 0, sk->pk.a_seed);

                polyr_ntt_cmul(y, vz[0], aij);
                for (j = 1; j < RACC_ELL; j++)
                {
                    expand_aij(aij, i, j, sk->pk.a_seed);
                    polyr_ntt_mula(y, vz[j], aij, y);
                }

                polyr_shlm(u, sk->pk.t[i], RACC_NUT, RACC_Q);
                polyr_fntt(u);
                polyr_ntt_cmul(u, u, c_poly);
                polyr2_sub(y, y, u);
                polyr_intt(y);
                polyr_reduce(y, y);

                //  --- 18. h := w - round( y )_q->q_w
                polyr_shrm44_asm(y, RACC_QW);
                polyr_subm(y, vw[i], y, RACC_QW);
                polyr_center(h, y, RACC_QW);
                // check bounds and decode on-the-fly
                racc_check_bounds_h(&h22, &hoo, h);
                l_sig = racc_encode_sig_h(sig, i, CRYPTO_BYTES, l_sig, &pre_z, &pre_k, h);
            }

            //  --- 19. sig := (c_hash, h, z)                   [caller]
            //  --- 20. if CheckBounds(sig) = FAIL goto Line 4
            rsp = racc_check_bounds_zh(z22, zoo, h22, hoo);

        } while (!rsp);

    } while (l_sig == 0);

    memset(sig + l_sig, 0, CRYPTO_BYTES - l_sig); //  zero padding
    //  --- 21. return sig                                  [caller]
    return 0;
}

#elif MEM_OPT == 1 // reduce the matrix stack usage; successful for raccoon_192_8, raccoon_256_4, raccoon_256_8
int racc_core_sign(uint8_t *sig, const uint8_t mu[RACC_MU_SZ],
                   racc_sk_t *sk)
{
    int i, j, k;
    int64_t *aij;
    int64_t mr[RACC_ELL][RACC_D][RACC_N];
    int64_t mw[RACC_D][RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t u[RACC_N], c_poly[RACC_N];
    int64_t z[RACC_ELL][RACC_N];
    int64_t *vz[RACC_ELL];
    int64_t(*h)[RACC_N];
    size_t l_sig = 0;
#if (RACC_D >= 2)
    int64_t *y;
    y = mr[0][1]; // only work for for RACC_D>=2
#else
    int64_t y[RACC_N];
#endif
    // skt = mw; // reuse mw for skt
    aij = u; // reuse u for aij
    h = vw;  // reuse vw for sig->h
    for (i = 0; i < RACC_ELL; i++)
    {
        vz[i] = mr[i][0]; // reuse mr for vz
    }

    bool rsp = false;
    mask_random_t mrg;
    //  --- 1.  (vk, [[s]]) := [[sk]], (seed, t) := vk      [ caller ]
    //  --- 2.  mu := H( H(vk) || msg )                     [ caller ]

    //  --- 3.  A := ExpandA(seed)
    do // move the racc_api do-while here.
    {
        //  intialize the mask random generator
        mask_random_init(&mrg);
        do
        {
            for (i = 0; i < RACC_ELL; i++)
            {
                //  --- 4.  [[r]] <- ZeroEncoding()
                zero_encoding(mr[i], &mrg);

                //  --- 5.  [[r]] <- AddRepNoise([[r]], uw, rep)
                add_rep_noise(mr[i], i, RACC_UW, &mrg);

                //  (Convert to NTT domain)
                for (j = 0; j < RACC_D; j++)
                {
                    polyr_fntt(mr[i][j]);
                }
            }

            for (i = 0; i < RACC_K; i++)
            {

                //  --- 6.  [[w]] := A * [[r]]
                for (j = 0; j < RACC_D; j++)
                {
                    expand_aij(aij, i, 0, sk->pk.a_seed);
                    polyr_ntt_cmul(mw[j], mr[0][j], aij);
                    for (k = 1; k < RACC_ELL; k++)
                    {
                        expand_aij(aij, i, k, sk->pk.a_seed);
                        polyr_ntt_mula(mw[j], mr[k][j], aij, mw[j]);
                    }
                    polyr_intt(mw[j]);
                }

                //  --- 7.  [[w]] <- AddRepNoise([[w]], uw, rep)
                add_rep_noise(mw, i, RACC_UW, &mrg);

                //  --- 8.  w := Decode([[w]])
                racc_decode(vw[i], mw);
                polyr_reduce(vw[i], vw[i]);
                //  --- 9.  w := round( w )_q->q_w
                polyr_shrm44_asm(vw[i], RACC_QW);
            }

            //  --- 10. c_hash := ChalHash(w, mu)
            xof_chal_hash(sig, mu, vw);
            l_sig = RACC_CH_SZ;

            //  --- 11. c_poly := ChalPoly(c_hash)
            xof_chal_poly(c_poly, sig);
            polyr_fntt(c_poly);

            for (i = 0; i < RACC_ELL; i++)
            {
                //  --- 12. [[s]] <- Refresh([[s]])
                racc_ntt_refresh_neg(sk->s[i], &mrg);

                //  --- 13. [[r]] <- Refresh([[r]])
                racc_ntt_refresh(mr[i], &mrg);

                //  --- 14. [[z]] := c_poly * [[s]] + [[r]]
                for (j = 0; j < RACC_D; j++)
                {
                    //  due to 2x Montgomery
                    polyr_ntt_smul(u, mr[i][j], MONT_RI1, MONT_RI2);
                    polyr_ntt_mula(mr[i][j], c_poly, sk->s[i][j], u);
                }

                //  --- 15. [[r]] <- Refresh([[r]])
                racc_ntt_refresh_neg(mr[i], &mrg);

                //  --- 16. z := Decode([[z]])
                racc_ntt_decode(z[i], mr[i]);
#if RACC_D > 2
                polyr2_reduce(z[i], z[i]);
#endif
                //  Two consecutive multiplications: Montgomery adjustment
                polyr_ntt_smul(vz[i], z[i], -MONT_RRR1, -MONT_RRR2);

                //  Decode for signature
                polyr_intt(z[i]);
                polyr_reduce(z[i], z[i]);
            }

            for (i = 0; i < RACC_K; i++)
            {
                //  --- 17. y := A*z - 2^{nu_t} * c_poly * t
                expand_aij(aij, i, 0, sk->pk.a_seed);
                polyr_ntt_cmul(y, aij, vz[0]);
                for (j = 1; j < RACC_ELL; j++)
                {
                    expand_aij(aij, i, j, sk->pk.a_seed);
                    polyr_ntt_mula(y, aij, vz[j], y);
                }
                polyr_shlm(u, sk->pk.t[i], RACC_NUT, RACC_Q);

                polyr_fntt(u);
                polyr_ntt_cmul(u, u, c_poly);
                polyr2_sub(y, y, u);
                polyr_intt(y);
                polyr_reduce(y, y);

                //  --- 18. h := w - round( y )_q->q_w
                polyr_shrm44_asm(y, RACC_QW);
                polyr_subm(y, vw[i], y, RACC_QW);
                polyr_center(h[i], y, RACC_QW);
            }

            //  --- 19. sig := (c_hash, h, z)                   [caller]

            //  --- 20. if CheckBounds(sig) = FAIL goto Line 4
            rsp = racc_check_bounds(h, z);

        } while (!rsp);

        l_sig = racc_encode_sig_zh(sig, CRYPTO_BYTES, h, z);

    } while (l_sig == 0);

    memset(sig + l_sig, 0, CRYPTO_BYTES - l_sig); //  zero padding
    //  --- 21. return sig                                  [caller]
    return 0;
}
#else
void racc_core_sign(racc_sig_t *sig, const uint8_t mu[RACC_MU_SZ],
                    racc_sk_t *sk)
{
    int i, j, k;
    int64_t ma[RACC_K][RACC_ELL][RACC_N];
    int64_t mr[RACC_ELL][RACC_D][RACC_N];
    int64_t mw[RACC_D][RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t y[RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t u[RACC_N], c_poly[RACC_N];
    bool rsp = false;
    mask_random_t mrg;

    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  (vk, [[s]]) := [[sk]], (seed, t) := vk      [ caller ]
    //  --- 2.  mu := H( H(vk) || msg )                     [ caller ]

    //  --- 3.  A := ExpandA(seed)
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_ELL; j++)
        {
            expand_aij(ma[i][j], i, j, sk->pk.a_seed);
        }
    }

    do
    {

        for (i = 0; i < RACC_ELL; i++)
        {

            //  --- 4.  [[r]] <- ZeroEncoding()
            zero_encoding(mr[i], &mrg);

            //  --- 5.  [[r]] <- AddRepNoise([[r]], uw, rep)
            add_rep_noise(mr[i], i, RACC_UW, &mrg);

            //  (Convert to NTT domain)
            for (j = 0; j < RACC_D; j++)
            {
                polyr_fntt(mr[i][j]);
            }
        }

        for (i = 0; i < RACC_K; i++)
        {

            //  --- 6.  [[w]] := A * [[r]]
            for (j = 0; j < RACC_D; j++)
            {
                polyr_ntt_cmul(mw[j], mr[0][j], ma[i][0]);
                for (k = 1; k < RACC_ELL; k++)
                {
                    polyr_ntt_mula(mw[j], mr[k][j], ma[i][k], mw[j]);
                }
                polyr_intt(mw[j]);
            }

            //  --- 7.  [[w]] <- AddRepNoise([[w]], uw, rep)
            add_rep_noise(mw, i, RACC_UW, &mrg);

            //  --- 8.  w := Decode([[w]])
            racc_decode(vw[i], mw);
            polyr_reduce(vw[i], vw[i]);

            //  --- 9.  w := round( w )_q->q_w
            polyr_shrm44_asm(vw[i], RACC_QW);
        }

        //  --- 10. c_hash := ChalHash(w, mu)
        xof_chal_hash(sig->ch, mu, vw);

        //  --- 11. c_poly := ChalPoly(c_hash)
        xof_chal_poly(c_poly, sig->ch);
        polyr_fntt(c_poly);

        for (i = 0; i < RACC_ELL; i++)
        {

            //  --- 12. [[s]] <- Refresh([[s]])
            racc_ntt_refresh_neg(sk->s[i], &mrg);

            //  --- 13. [[r]] <- Refresh([[r]])
            racc_ntt_refresh(mr[i], &mrg);

            //  --- 14. [[z]] := c_poly * [[s]] + [[r]]
            for (j = 0; j < RACC_D; j++)
            {
                //  due to 2x Montgomery
                polyr_ntt_smul(u, mr[i][j], MONT_RI1, MONT_RI2);
                polyr_ntt_mula(mr[i][j], c_poly, sk->s[i][j], u);
            }

            //  --- 15. [[r]] <- Refresh([[r]])
            racc_ntt_refresh_neg(mr[i], &mrg);

            //  --- 16. z := Decode([[z]])
            racc_ntt_decode(sig->z[i], mr[i]);
#if RACC_D > 2
            polyr2_reduce(sig->z[i], sig->z[i]);
#endif
            //  Two consecutive multiplications: Montgomery adjustment
            polyr_ntt_smul(vz[i], sig->z[i], -MONT_RRR1, -MONT_RRR2);

            //  Decode for signature
            polyr_intt(sig->z[i]);
            polyr_reduce(sig->z[i], sig->z[i]);
        }

        for (i = 0; i < RACC_K; i++)
        {

            //  --- 17. y := A*z - 2^{nu_t} * c_poly * t
            polyr_ntt_cmul(y, ma[i][0], vz[0]);
            for (j = 1; j < RACC_ELL; j++)
            {
                polyr_ntt_mula(y, ma[i][j], vz[j], y);
            }

            polyr_shlm(u, sk->pk.t[i], RACC_NUT, RACC_Q);
            polyr_fntt(u);
            polyr_ntt_cmul(u, u, c_poly);
            polyr2_sub(y, y, u);
            polyr_intt(y);
            polyr_reduce(y, y);

            //  --- 18. h := w - round( y )_q->q_w
            polyr_shrm44_asm(y, RACC_QW);
            polyr_subm(y, vw[i], y, RACC_QW);
            polyr_center(sig->h[i], y, RACC_QW);
        }

        //  --- 19. sig := (c_hash, h, z)                   [caller]

        //  --- 20. if CheckBounds(sig) = FAIL goto Line 4
        rsp = racc_check_bounds(sig->h, sig->z);

    } while (!rsp);

    //  --- 21. return sig                                  [caller]
}
#endif

//  === racc_core_verify ===
//  Verify that the signature "sig" is valid for digest "mu".
//  Returns true iff signature is valid, false if not valid.
bool racc_core_verify(const racc_sig_t *sig,
                      const uint8_t mu[RACC_MU_SZ],
                      const racc_pk_t *pk)
{
    int i, j;
    int64_t aij[RACC_N];
    int64_t c_poly[RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t t[RACC_N], u[RACC_N];
    uint8_t c_hchk[RACC_CH_SZ];

    //  --- 1.  (c hash, h, z) := sig, (seed, t) := vk      [caller]

    //  --- 2.  if CheckBounds(sig) = FAIL return FAIL
    if (!racc_check_bounds(sig->h, sig->z))
    {
        return false;
    }
    //  --- 3.  mu := H( H(vk) || msg )                     [caller]

    //  --- 5.  c_poly := ChalPoly(c_hash)
    xof_chal_poly(c_poly, sig->ch);
    polyr_fntt(c_poly);

    for (i = 0; i < RACC_ELL; i++)
    {
        polyr_copy(vz[i], sig->z[i]);
        polyr_fntt(vz[i]);
    }

    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_ELL; j++)
        {

            //  --- 4.  A := ExpandA(seed)
            expand_aij(aij, i, j, pk->a_seed);

            //  --- 6.  y = A * z - 2^{nu_t} * c_poly * t
            if (j == 0)
            {
                polyr_ntt_cmul(t, aij, vz[0]);
            }
            else
            {
                polyr_ntt_mula(t, aij, vz[j], t);
            }
        }

        polyr_shlm(u, pk->t[i], RACC_NUT, RACC_Q); //  .. - p_t * t ..
        polyr_fntt(u);
        polyr_ntt_cmul(u, u, c_poly); //  .. Cpoly ..
        polyr2_sub(vw[i], t, u);
        polyr_intt(vw[i]);
        polyr_reduce(vw[i], vw[i]);

        //  --- 7.  w' = round( y )_q->q_w + h
        polyr_shrm44_asm(vw[i], RACC_QW);
        polyr_nonneg(u, sig->h[i], RACC_QW);
        polyr_addm(vw[i], vw[i], u, RACC_QW);
    }

    //  --- 8. c_hash' := ChalHash(w', mu)
    xof_chal_hash(c_hchk, mu, vw);
    //  --- 9. if c_hash != c_hash' return FAIL
    //  --- 10. (else) return OK
    return ct_equal(c_hchk, sig->ch, RACC_CH_SZ);
}
