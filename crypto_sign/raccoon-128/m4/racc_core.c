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
// #define XDEBUG 1
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

void expand_aij(int64_t aij[RACC_N], int i_k, int i_ell,
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
    polyr_addq(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++)
    {
        polyr_addq(r, r, m[i]);
    }
#endif
}

//  Decode(): Collapse shares (possibly split CRT arithmetic)
// ||r||*d
void racc_ntt_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N])
{
#if RACC_D == 1
    polyr_copy(r, m[0]);
#else
    int i;
    polyr2_addq(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++)
    {
        polyr2_addq(r, r, m[i]);
    }
#endif
}

//  ZeroEncoding(d) -> [[z]]d
//  in-place version
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
        polyr_negm(z[i + 1], z[i], RACC_Q);
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
                polyr_addq(z[j], z[j], r);
                polyr_subq(z[j + d], z[j + d], r);
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
        polyr_addq(x[i], x[i], z[i]);
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
        polyr2_addq(x[i], x[i], z[i]);
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
        polyr2_subq(x[i], z[i], x[i]);
    }
#endif
}

#if MEM_OPT==2
// NI-secure mask compression implementation.
// compress x 

void racc_mask_compress(poly_mask_compress_t *r, int64_t x[RACC_D][RACC_N], int i_ell)
{
    polyr2_neg(r->x0, x[0]);
    polyr2_join(r->x0, MONT_D2Q1, MONT_D2Q2);
#if RACC_D>1
    int64_t t[RACC_N];
    uint8_t buf[RACC_MK_SZ + 8];
    memset(buf, 0x00, 8); //   domain header template
    buf[0]='K';
    for(int i=1;i<RACC_D;i++){
        randombytes(r->z[i-1], RACC_MK_SZ);
        memcpy(buf+8, r->z[i-1], RACC_MK_SZ);

        buf[1] = i_ell;// update domain header
        buf[2] = i;
        xof_sample_q(t, buf, RACC_MK_SZ + 8);
        polyr_subq(r->x0, r->x0, t);// s0<-s0-r
        polyr2_neg(t, x[i]);
        polyr2_join(t, MONT_D2Q1, MONT_D2Q2);
        polyr_addq(r->x0, r->x0, t); // s0<-s0+s_i
    }
#endif
}

// Get i-th share of the mask. Output in negative & NTT domain.
void racc_load_share_neg(int64_t r[RACC_N], poly_mask_compress_t *x, int i_d, int i_ell)
{
    if (i_d==0){
        polyr_copy(r, x->x0); 
        polyr2_split_neg(r);
    }
    else{
        int64_t t[RACC_N];
        uint8_t buf[RACC_MK_SZ + 8];
        memset(buf, 0x00, 8); //   domain header template
        buf[0]='K';
        buf[1] = i_ell;
        buf[2] = i_d;
        memcpy(buf+8, x->z[i_d-1], RACC_MK_SZ);
        xof_sample_q(r, buf, RACC_MK_SZ + 8);

        randombytes(x->z[i_d - 1], RACC_MK_SZ);
        memcpy(buf + 8, x->z[i_d - 1], RACC_MK_SZ);
        xof_sample_q(t, buf, RACC_MK_SZ + 8);
        polyr_subq(x->x0, x->x0, t);
        polyr_addq(x->x0, x->x0, r);

        polyr2_split_neg(r);
    }
}

void racc_load_share(int64_t r[RACC_N], poly_mask_compress_t *x, int i_d, int i_ell)
{
    if (i_d == 0)
    {
        polyr_copy(r, x->x0);
        polyr2_split(r);
    }
    else
    {
        int64_t t[RACC_N];
        uint8_t buf[RACC_MK_SZ + 8];
        memset(buf, 0x00, 8); //   domain header template
        buf[0] = 'K';
        buf[1] = i_ell;
        buf[2] = i_d;
        memcpy(buf + 8, x->z[i_d - 1], RACC_MK_SZ);
        xof_sample_q(r, buf, RACC_MK_SZ + 8);

        randombytes(x->z[i_d - 1], RACC_MK_SZ);
        memcpy(buf + 8, x->z[i_d - 1], RACC_MK_SZ);
        xof_sample_q(t, buf, RACC_MK_SZ + 8);
        polyr_subq(x->x0, x->x0, t);
        polyr_addq(x->x0, x->x0, r);

        polyr2_split(r);
    }
}

void racc_load_fullshare(int64_t r[RACC_D][RACC_N], poly_mask_compress_t *x, int i_ell)
{
    int i;
    for (i = 0; i < RACC_D; i++)
    {
        racc_load_share(r[i], x, i, i_ell);
    }
}

void racc_mask_refresh(poly_mask_compress_t* r, int i_ell)
{
    int i, j;
    int64_t t[RACC_N], t2[RACC_N];
    uint8_t buf[RACC_MK_SZ + 8];
    memset(buf, 0x00, 8); //   domain header template
    buf[0] = 'K';
    for (i = 0; i < RACC_D; i++)
    {
        for (j = 1; j < RACC_D; j++)
        {
            buf[1] = i_ell;// update domain header
            buf[2] = j;
            memcpy(buf + 8, r->z[j - 1], RACC_MK_SZ);
            xof_sample_q(t, buf, RACC_MK_SZ + 8);

            randombytes(r->z[j - 1], RACC_MK_SZ);
            memcpy(buf + 8, r->z[j - 1], RACC_MK_SZ);
            xof_sample_q(t2, buf, RACC_MK_SZ + 8);
            polyr_subq(r->x0, r->x0, t2);
            polyr_addq(r->x0, r->x0, t);
        }
    }
}
#endif

#if MEM_OPT > 0
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
            polyr_addq(vi[j], vi[j], r);
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
            polyr_addq(vi[j], vi[j], r);
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
#if MEM_OPT > 0
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
    bool ret=true;
    //  --- 3:  if ||h||oo > round(Boo/2^nuw) return FAIL
    if (hoo > ((int64_t)(RACC_BOO + ((int64_t)1l << (RACC_NUW - 1))) >> RACC_NUW))
    {
        ret=false;
    }

    //  --- 4.  if ||z||oo > Boo return FAIL
    if (zoo > RACC_BOO)
    {
        ret=false;
    }

    //  --- 5.  h2 := 2^(2*nuw - 64) * ||h||^2
    //  --- 7.  if (h2 + z2) > 2^-64*B22 return FAIL
    if (((h22 << (2 * RACC_NUW - 64)) + z22) > RACC_B22)
    {
        ret=false;
    }

    //  --- 8.  return OK
    return ret;
}
#endif

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
int racc_core_keygen(unsigned char *pk, racc_sk_compress_t *sk)
{
    int i, j, k;
    size_t l_pk = 0;
    int64_t aij[RACC_N];
    int64_t mt[RACC_D][RACC_N], mw[RACC_D][RACC_N];
    int64_t t[RACC_N];
    uint8_t seed[RACC_AS_SZ + RACC_ELL * RACC_D * RACC_REP * RACC_SEC + RACC_K * RACC_D * RACC_REP * RACC_SEC];
    uint8_t *seed_mt= seed + RACC_AS_SZ;
    uint8_t *seed_mw= seed_mt + RACC_ELL * RACC_D * RACC_REP * RACC_SEC;

    mask_random_t mrg;
    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  seed <- {0,1}^kappa
    randombytes(seed, sizeof(seed));

    memcpy(pk, seed, RACC_AS_SZ);
    //  encode A seed
    l_pk += RACC_AS_SZ;

    for (i = 0; i < RACC_K; i++)
    {
        //  --- 5.  [[t]] := A * [[s]]
        //  --- 2.  A := ExpandA(seed)
        expand_aij(aij, i, 0, pk);
        //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
        zero_encoding(mt, &mrg);

        //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
        add_rep_noise_buf(mt, 0, RACC_UT, seed + RACC_AS_SZ + 0 * RACC_REP * RACC_D * RACC_SEC, &mrg);
        for (j = 0; j < RACC_D; j++)
        {
            polyr_fntt(mt[j]);
            polyr_ntt_cmul(mw[j], mt[j], aij);
        }
        if(i==0){
            racc_mask_compress(&sk->s[0], mt, 0); // store the compressed mask in sk->s[0]
        }

        for (k = 1; k < RACC_ELL; k++)
        {
            expand_aij(aij, i, k, pk);
            //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
            zero_encoding(mt, &mrg);

            //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
            add_rep_noise_buf(mt, k, RACC_UT, seed + RACC_AS_SZ + k * RACC_REP * RACC_D * RACC_SEC, &mrg);
            for (j = 0; j < RACC_D; j++)
            {
                polyr_fntt(mt[j]);
                polyr_ntt_mula(mw[j], mt[j], aij, mw[j]);
            }
            if (i == 0)
            {
                racc_mask_compress(&sk->s[k], mt, k); // store the compressed mask in sk->s[k]
            }
        }
        for (j = 0; j < RACC_D; j++)
        {
            polyr_intt(mw[j]);
        }

        //  --- 6.  [[t]] <- AddRepNoise([[t]], ut, rep)
        add_rep_noise_buf(mw, i, RACC_UT, seed_mw + i * RACC_D * RACC_REP * RACC_SEC, &mrg);

        //  --- 7.  t := Decode([[t]])
        racc_decode(t, mw);
        //  --- 8.  t := round( t_m )_q->q_t
        polyr_shrm42_asm(t, RACC_QT); // 49-42=7-bit

        // --- encode pk on-the-fly
        l_pk = racc_encode_pk_k(pk, t, l_pk);
    }

    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    return (l_pk != CRYPTO_PUBLICKEYBYTES);
}
#elif MEM_OPT == 1
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
                   racc_sk_compress_t *sk)
{
    int i, j, k;
    int64_t *aij;
    racc_sk_compress_t mr;
    // int64_t mr[RACC_D][RACC_N];
    int64_t mw[RACC_D][RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t u[RACC_N], c_poly[RACC_N], y[RACC_N], *h, *z;
    uint8_t seed[RACC_AS_SZ];
    size_t l_sig, pre_k, l_pk;
    uint8_t pre_z;
    int64_t h22, hoo, z22, zoo;

    aij = u; // reuse u for aij
    z = u;   // reuse u for z
    h = u;   // reuse u for h

    bool rsp = false;
    mask_random_t mrg;
    //  --- 1.  (vk, [[s]]) := [[sk]], (seed, t) := vk      [ caller ]
    //  --- 2.  mu := H( H(vk) || msg )                     [ caller ]

    // --- get pk.a_seed
    memcpy(seed, sk->pk, RACC_AS_SZ);

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
            for(i=0; i< RACC_ELL; i++)
            {
                //  --- 4.  [[r]] <- ZeroEncoding()
                zero_encoding(mw, &mrg);

                //  --- 5.  [[r]] <- AddRepNoise([[r]], uw, rep)
                add_rep_noise(mw, i, RACC_UW, &mrg);

                //  (Convert to NTT domain)
                for (j = 0; j < RACC_D; j++)
                {
                    polyr_fntt(mw[j]);
                }
                racc_mask_compress(&mr.s[i], mw, i); // compress the mask polynomials and store the random seed for reconstructing the mask
            }

            for (i = 0; i < RACC_K; i++)
            {
                //  --- 3.  A := ExpandA(seed)
                //  --- 6.  [[w]] := A * [[r]]
                expand_aij(aij, i, 0, seed);
                
                for (j = 0; j < RACC_D; j++)
                {
                    racc_load_share_neg(c_poly, &mr.s[0], j, 0);
                    polyr_ntt_cmul(mw[j], c_poly, aij);
                }
                for (k = 1; k < RACC_ELL; k++)
                {
                    expand_aij(aij, i, k, seed);

                    for (j = 0; j < RACC_D; j++)
                    {
                        racc_load_share_neg(c_poly, &mr.s[k], j, k);
                        polyr_ntt_mula(mw[j], c_poly, aij, mw[j]);
                    }
                }
                for (j = 0; j < RACC_D; j++)
                {
                    polyr_intt(mw[j]);
                }

                for (j = 0; j < RACC_ELL; j++)
                {
                    racc_mask_refresh(&mr.s[j], j);
                }
                //  --- 7.  [[w]] <- AddRepNoise([[w]], uw, rep)
                add_rep_noise(mw, i, RACC_UW, &mrg);

                //  --- 8.  w := Decode([[w]])
                racc_decode(vw[i], mw);
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
                racc_mask_refresh(&sk->s[i], i);

                //  --- 13. [[r]] <- Refresh([[r]])
                racc_mask_refresh(&mr.s[i], i);

                //  --- 14. [[z]] := c_poly * [[s]] + [[r]]
                for (j = 0; j < RACC_D; j++)
                {
                    //  due to 2x Montgomery
                    racc_load_share(y, &sk->s[i], j, i);
                    racc_load_share_neg(u, &mr.s[i], j, i);
                    polyr_ntt_smul(u, u, MONT_RI1, MONT_RI2);
                    polyr_ntt_mula(mw[j], c_poly, y, u);
                    polyr2_full_reduce(mw[j], mw[j]);
                }

                //  --- 15. [[r]] <- Refresh([[r]])
                racc_ntt_refresh_neg(mw, &mrg);

                //  --- 16. z := Decode([[z]])
                racc_ntt_decode(z, mw);

                //  Two consecutive multiplications: Montgomery adjustment
                polyr_ntt_smul(vz[i], z, -MONT_RRR1, -MONT_RRR2); // negative & normal domain

                //  Decode for signature
                polyr_intt(z);
                // check bounds and decode on-the-fly
                racc_check_bounds_z(&z22, &zoo, z);
                l_sig = racc_encode_sig_z(sig, CRYPTO_BYTES, l_sig, &pre_z, &pre_k, z);
            }

            l_pk = RACC_AS_SZ;
            for (i = 0; i < RACC_K; i++)
            {
                //  --- 17. y := A*z - 2^{nu_t} * c_poly * t
                expand_aij(aij, i, 0, seed);

                polyr_ntt_cmul(y, vz[0], aij);
                for (j = 1; j < RACC_ELL; j++)
                {
                    expand_aij(aij, i, j, seed);
                    polyr_ntt_mula(y, vz[j], aij, y);
                }
                // generate pk.t on-the-fly to save memory
                l_pk=racc_decode_pk_k(mw[0], sk->pk, l_pk);
                polyr_shlm(u, mw[0], RACC_NUT, RACC_Q);

                polyr_fntt(u);
                polyr_ntt_cmul(u, u, c_poly);
                polyr2_sub(y, y, u);
                polyr_intt(y);

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

#elif MEM_OPT == 1 
// signature serialization on-the-fly, and streaming the masked noise from precomputed buffer
int racc_core_sign(uint8_t *sig, const uint8_t mu[RACC_MU_SZ], racc_sk_t *sk)
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
                    polyr2_full_reduce(mr[j], mr[j]);
                }

                //  --- 15. [[r]] <- Refresh([[r]])
                racc_ntt_refresh_neg(mr, &mrg);

                //  --- 16. z := Decode([[z]])
                racc_ntt_decode(z, mr);

                //  Two consecutive multiplications: Montgomery adjustment
                polyr_ntt_smul(vz[i], z, -MONT_RRR1, -MONT_RRR2); // negative & normal domain

                //  Decode for signature
                polyr_intt(z);
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
                polyr2_full_reduce(mr[i][j], mr[i][j]);
            }

            //  --- 15. [[r]] <- Refresh([[r]])
            racc_ntt_refresh_neg(mr[i], &mrg);

            //  --- 16. z := Decode([[z]])
            racc_ntt_decode(sig->z[i], mr[i]);

            //  Two consecutive multiplications: Montgomery adjustment
            polyr_ntt_smul(vz[i], sig->z[i], -MONT_RRR1, -MONT_RRR2);

            //  Decode for signature
            polyr_intt(sig->z[i]);
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

#if MEM_OPT > 0
bool racc_core_verify(const uint8_t *sig,
                      const uint8_t mu[RACC_MU_SZ],
                      const uint8_t *pk)
{
    int i, j, l_pk, l_sig;
    bool rsp = true;
    uint8_t pre_z=0;
    size_t pre_k=0;
    int64_t h22, hoo, z22, zoo;

    uint8_t seed[RACC_AS_SZ], chal[RACC_CH_SZ];
    int64_t aij[RACC_N];
    int64_t c_poly[RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t t[RACC_N], u[RACC_N];
    uint8_t c_hchk[RACC_CH_SZ];

    memcpy(seed, pk, RACC_AS_SZ);
    memcpy(chal, sig, RACC_CH_SZ);
    l_pk=RACC_AS_SZ; // for on-the-fly decoding of pk.t
    l_sig=RACC_CH_SZ; // for on-the-fly decoding of sig.z and sig.h

    h22=0;
    hoo=0;
    z22=0;
    zoo=0;

    //  --- 1.  (c hash, h, z) := sig, (seed, t) := vk      [caller]
    //  --- 3.  mu := H( H(vk) || msg )                     [caller]

    //  --- 5.  c_poly := ChalPoly(c_hash)
    xof_chal_poly(c_poly, chal);
    polyr_fntt(c_poly);

    for (i = 0; i < RACC_ELL; i++)
    {
        l_sig = racc_decode_sig_z(vz[i], RACC_SIG_SZ, l_sig, &pre_z, &pre_k, sig); // decode sig.z on-the-fly to save memory
        racc_check_bounds_z(&z22, &zoo, vz[i]);
        polyr_fntt(vz[i]);
    }
    
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_ELL; j++)
        {

            //  --- 4.  A := ExpandA(seed)
            expand_aij(aij, i, j, seed);

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

        l_pk = racc_decode_pk_k(aij, pk, l_pk); // decode pk.t on-the-fly to save memory
        polyr_shlm(u, aij, RACC_NUT, RACC_Q);   //  .. - p_t * t ..
        polyr_fntt(u);
        polyr_ntt_cmul(u, u, c_poly); //  .. Cpoly ..
        polyr2_sub(vw[i], t, u);
        polyr_intt(vw[i]);

        //  --- 7.  w' = round( y )_q->q_w + h
        polyr_shrm44_asm(vw[i], RACC_QW);

        l_sig = racc_decode_sig_h(aij, RACC_SIG_SZ, l_sig, &pre_z, &pre_k, sig); // decode sig.h on-the-fly to save memory
        racc_check_bounds_h(&h22, &hoo, aij);
        polyr_nonneg(u, aij, RACC_QW);
        polyr_addm(vw[i], vw[i], u, RACC_QW);
    }

    // check zero padding in signature
    if (pre_k > 0 && (pre_z >> pre_k) != 0)
        l_sig = 0;

    while (l_sig < RACC_SIG_SZ)
    {
        if (sig[l_sig++] != 0)
        {
            l_sig = 0;
            break;
        }
    }
    rsp = racc_check_bounds_zh(z22, zoo, h22, hoo);

    if(l_pk != CRYPTO_PUBLICKEYBYTES || l_sig != CRYPTO_BYTES || !rsp)
    {
        return false;
    }

    //  --- 8. c_hash' := ChalHash(w', mu)
    xof_chal_hash(c_hchk, mu, vw);
    //  --- 9. if c_hash != c_hash' return FAIL
    //  --- 10. (else) return OK
    return ct_equal(c_hchk, chal, RACC_CH_SZ);
}

#else
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
#endif