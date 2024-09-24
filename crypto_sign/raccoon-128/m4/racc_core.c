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

#define XDEBUG 1
#ifdef XDEBUG
#include <stdio.h>
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
#endif

//  ExpandA(): Use domain separated XOF to create matrix elements

static void expand_aij( int64_t aij[RACC_N], int i_k, int i_ell,
                        const uint8_t seed[RACC_AS_SZ])
{
    uint8_t buf[RACC_AS_SZ + 8];

    //  --- 3.  hdrA := Ser8(65, i, j, 0, 0, 0, 0, 0)
    buf[0] = 'A';       //  ascii 65
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

static void racc_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N])
{
#if RACC_D == 1
    polyr_copy(r, m[0]);
#else
    int i;

    polyr_addq(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++) {
        polyr_addq(r, r, m[i]);
    }
#endif
}

//  Decode(): Collapse shares (possibly split CRT arithmetic)

static void racc_ntt_decode(int64_t r[RACC_N], const int64_t m[RACC_D][RACC_N])
{
#if RACC_D == 1
    polyr_copy(r, m[0]);
#else
    int i;

    polyr_ntt_addq(r, m[0], m[1]);
    for (i = 2; i < RACC_D; i++) {
        polyr_ntt_addq(r, r, m[i]);
    }
#endif
}

//  ZeroEncoding(d) -> [[z]]d
//  in-place version

static void zero_encoding(int64_t z[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void) mrg;
    polyr_zero(z[0]);
#else
    int i, j, d;
    int64_t r[RACC_N];

    //  d = 2
    for (i = 0; i < RACC_D; i += 2) {
        mask_random_poly(mrg, z[i], i);
        polyr_negm(z[i + 1], z[i], RACC_Q);
    }

    //  d = 4, 8, ..
    d = 2;
    while (d < RACC_D) {
        for (i = 0; i < RACC_D; i += 2 * d) {
            for (j = i; j < i + d; j++) {
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

static void racc_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void) x;
    (void) mrg;
#else
    int i;
    int64_t z[RACC_D][RACC_N];

    //  --- 1.  [[z]] <- ZeroEncoding(d)
    zero_encoding(z, mrg);

    //  --- 2.  return [[x]]' := [[x]] + [[z]]
    for (i = 0; i < RACC_D; i++) {
        polyr_addq(x[i], x[i], z[i]);
    }
#endif
}

//  Refresh([[x]]) -> [[x]]′ ( NTT domain )

static void racc_ntt_refresh(int64_t x[RACC_D][RACC_N], mask_random_t *mrg)
{
#if RACC_D == 1
    (void) x;
    (void) mrg;
#else
    int i;
    int64_t z[RACC_D][RACC_N];

    //  --- 1.  [[z]] <- ZeroEncoding(d)
    zero_encoding(z, mrg);

    //  --- 2.  return [[x]]' := [[x]] + [[z]]
    for (i = 0; i < RACC_D; i++) {
        polyr2_split(z[i]);
        polyr_ntt_addq(x[i], x[i], z[i]);
    }
#endif
}

//  AddRepNoise([[v]], u, rep) -> [[v]]
//  Add repeated noise to a polynomial (vector at index i_v)

static void add_rep_noise(  int64_t vi[RACC_D][RACC_N],
                            int i_v, int u, mask_random_t *mrg)
{
    int i_rep, j;
    uint8_t buf[RACC_SEC + 8];
    int64_t r[RACC_N];

    //  --- 1.  for i in [len(v)] do                        [caller]

    //  --- 2.  for i_rep in [rep] do
    for (i_rep = 0; i_rep < RACC_REP; i_rep++) {

        //  --- 3.  for j in [d] do:
        for (j = 0; j < RACC_D; j++) {

            //  --- 4.  sigma <- {0,1}^kappa
            randombytes(buf + 8, RACC_SEC);

            //  --- 5.  hdr_u := Ser8('u' || i_rep || i_v || j || (0) || seed)
            buf[0] = 'u';       //  ascii 117
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
#ifdef MEM_OPT
// use the noise in precomputed buffer
static void add_rep_noise_buf(int64_t vi[RACC_D][RACC_N],
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

        //  --- [[v_i]] <- Refresh([[v_i]])
        racc_refresh(vi, mrg);
    }
}
#endif
//  "rounding" shift right

static inline void round_shift_r(int64_t *r, int64_t q, int s)
{
    int i;
    int64_t x, rc;

    rc = 1ll << (s - 1);
    for (i = 0; i < RACC_N; i++) {
        x = (r[i] + rc) >> s;
        r[i] = mont64_csub(x, q);
    }
}

//  CheckBounds(sig) -> {OK or FAIL}

static bool racc_check_bounds(  const int64_t h[RACC_K][RACC_N],
                                const int64_t z[RACC_ELL][RACC_N])
{
    int i, j;
    int64_t x, h22, hoo, z22, zoo;

    //  --- 1.  if |sig| != |sig|default return FAIL        [caller]
    //  --- 2.  (c hash, h, z) := sig                       [caller]

    //  Infinity and L2 norms for hint
    h22 = 0;
    hoo = 0;
    for (i = 0; i < RACC_K; i++) {
        for (j = 0; j < RACC_N; j++) {
            x = h[i][j];
            if (x < 0)                      //  x mod q  (non-negative)
                x = -x;
            if (x > hoo)
                hoo = x;
            h22 += (x * x);
        }
    }

    //  Infinity norm and scaled L2 norm for z
    z22 = 0;
    zoo = 0;
    for (i = 0; i < RACC_ELL; i++) {
        for (j = 0; j < RACC_N; j++) {
            x = z[i][j];
            if (x < 0)                      //  x mod q  (non-negative)
                x += RACC_Q;
            if (x > (RACC_Q / 2))           //  absolute value
                x = RACC_Q - x;
            if (x > zoo)
                zoo = x;

            //  --- 6.  z2 := sum_i [ abs(zi) / 2^32 ]^2
            x >>= 32;                       //  scale to avoid overflow
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
#ifdef MEM_OPT
int racc_core_keygen(unsigned char *pk, unsigned char *sk)
{
    int i, j, k;
    size_t l_pk = 0, l_sk = 0, l_sk_key = 0;
    int64_t aij[RACC_N];
    int64_t mt[RACC_D][RACC_N];
    int64_t skt[RACC_D][RACC_N];
    int64_t *t;
    uint8_t seed_buf[RACC_AS_SZ + RACC_ELL * RACC_D * RACC_REP * RACC_SEC + RACC_K * RACC_D * RACC_REP * RACC_SEC + (RACC_D - 1) * RACC_MK_SZ];
    uint8_t *seed, *seed_sk, *seed_mt, *seed_key;
    t = skt[0];

    seed = seed_buf;
    seed_sk = seed + RACC_AS_SZ;
    seed_mt = seed_sk + RACC_ELL * RACC_REP * RACC_D * RACC_SEC;
    seed_key = seed_mt + RACC_K * RACC_D * RACC_REP * RACC_SEC;

    mask_random_t mrg;
    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  seed <- {0,1}^kappa; and noise for sk, mt, and decode_sk.
    randombytes(seed_buf, sizeof(seed_buf));

    //  encode A seed
    memcpy(pk + l_pk, seed, RACC_AS_SZ);
    l_pk += RACC_AS_SZ;
    l_sk += CRYPTO_PUBLICKEYBYTES;
    l_sk_key = l_sk;
    l_sk += (RACC_D - 1) * RACC_MK_SZ;

    // encode sk seed
    memcpy(sk + l_sk_key, seed_key, (RACC_D - 1) * RACC_MK_SZ);

    for (i = 0; i < RACC_K; i++)
    {
        //  --- 2.  A := ExpandA(seed)
        expand_aij(aij, i, 0, seed);
        //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
        zero_encoding(skt, &mrg);

        //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
        add_rep_noise_buf(skt, 0, RACC_UT, seed_sk + 0 * RACC_REP * RACC_D * RACC_SEC, &mrg);

        //  --- 5.  [[t]] := A * [[s]]

        for (j = 0; j < RACC_D; j++)
        {
            polyr_fntt(skt[j]);
            polyr_ntt_cmul(mt[j], skt[j], aij);
        }

        if (i == 0)
            l_sk = racc_encode_sk_l(sk, skt, 0, l_sk_key, l_sk);

        for (k = 1; k < RACC_ELL; k++)
        {
            expand_aij(aij, i, k, seed);

            //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
            zero_encoding(skt, &mrg);

            //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
            add_rep_noise_buf(skt, k, RACC_UT, seed_sk + k * RACC_REP * RACC_D * RACC_SEC, &mrg);

            for (j = 0; j < RACC_D; j++)
            {
                polyr_fntt(skt[j]);
                polyr_ntt_mula(mt[j], skt[j], aij, mt[j]);
            }

            if (i == 0)
                l_sk = racc_encode_sk_l(sk, skt, k, l_sk_key, l_sk);
        }
        for (j = 0; j < RACC_D; j++)
        {
            polyr_intt(mt[j]);
        }

        //  --- 6.  [[t]] <- AddRepNoise([[t]], ut, rep)
        add_rep_noise_buf(mt, i, RACC_UT, seed_mt + i * RACC_REP * RACC_D * RACC_SEC, &mrg);

        //  --- 7.  t := Decode([[t]])
        racc_decode(t, mt);

        //  --- 8.  t := round( t_m )_q->q_t
        round_shift_r(t, RACC_QT, RACC_NUT); // 49-42=7-bit

        // --- encode pk on-the-fly
        l_pk = racc_encode_pk_k(pk, t, l_pk);
    }
    memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);
    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    if (l_pk != CRYPTO_PUBLICKEYBYTES || l_sk != CRYPTO_SECRETKEYBYTES)
        return -1;
    return 0;
}
#elif defined(MEM_OPT1)
int racc_core_keygen(unsigned char *pk, unsigned char *sk)
{
    int i, j, k;
    size_t l_pk = 0, l_sk = 0, l_sk_key = 0;
    int64_t ai[RACC_K][RACC_N];
    int64_t mt[RACC_D][RACC_N];
    int64_t skt[RACC_ELL][RACC_D][RACC_N];
    int64_t *t;
    uint8_t seed[RACC_AS_SZ];
    t = mt[0];

    mask_random_t mrg;
    //  intialize the mask random generator
    mask_random_init(&mrg);

    //  --- 1.  seed <- {0,1}^kappa
    randombytes(seed, RACC_AS_SZ);

    //  encode A seed
    memcpy(pk, seed, RACC_AS_SZ);
    l_pk += RACC_AS_SZ;

    for (i = 0; i < RACC_ELL; i++)
    {
        //  --- 3.  [[s]] <- ell * ZeroEncoding(d)
        zero_encoding(skt[i], &mrg);

        //  --- 4.  [[s]] <- AddRepNoise([[s]], ut, rep)
        add_rep_noise(skt[i], i, RACC_UT, &mrg);

        for (j = 0; j < RACC_D; j++)
        {
            polyr_fntt(skt[i][j]);
        }
    }

    for (i = 0; i < RACC_K; i++)
    {
        //  --- 2.  A := ExpandA(seed)
        for(j=0;j<RACC_ELL;j++){
            expand_aij(ai[j], i, j, seed);
        }

        //  --- 5.  [[t]] := A * [[s]]
        for (j = 0; j < RACC_D; j++)
        {
            polyr_ntt_cmul(mt[j], skt[0][j], ai[0]);
            for (k = 1; k < RACC_ELL; k++)
            {
                polyr_ntt_mula(mt[j], skt[k][j], ai[k], mt[j]);
            }
            polyr_intt(mt[j]);
        }

        //  --- 6.  [[t]] <- AddRepNoise([[t]], ut, rep)
        add_rep_noise(mt, i, RACC_UT, &mrg);

        //  --- 7.  t := Decode([[t]])
        racc_decode(t, mt);

        //  --- 8.  t := round( t_m )_q->q_t
        round_shift_r(t, RACC_QT, RACC_NUT); // 49-42=7-bit

        // --- encode pk on-the-fly
        l_pk = racc_encode_pk_k(pk, t, l_pk);
    }

    // --- encode sk
    memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);
    l_sk += CRYPTO_PUBLICKEYBYTES;
    l_sk_key = l_sk;
    randombytes(sk + l_sk, (RACC_D - 1) * RACC_MK_SZ);
    l_sk += (RACC_D - 1) * RACC_MK_SZ;

    for(i=0; i<RACC_ELL; i++){
        l_sk = racc_encode_sk_l(sk, skt[i], i, l_sk_key, l_sk);
    }

    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    if (l_pk != CRYPTO_PUBLICKEYBYTES || l_sk != CRYPTO_SECRETKEYBYTES)
        return -1;
    return 0;
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
        round_shift_r(pk->t[i], RACC_QT, RACC_NUT);
    }

    //  --- 9.  return ( (vk := seed, t), sk:= (vk, [[s]]) )
    memcpy(&sk->pk, pk, sizeof(racc_pk_t));
}
#endif

//  === racc_core_sign ===
//  Create a detached signature "sig" for digest "mu" using secret key "sk".
#if defined(MEM_OPT) || defined(MEM_OPT1)
void racc_core_sign(uint8_t *sig, const uint8_t mu[RACC_MU_SZ],
                    uint8_t *sk)
{
    int i, j, k;
    int64_t ma[RACC_N];
    int64_t mr[RACC_ELL][RACC_D][RACC_N];
    int64_t mw[RACC_D][RACC_N];
    int64_t vw[RACC_K][RACC_N];
    int64_t y[RACC_N];
    int64_t vz[RACC_ELL][RACC_N];
    int64_t u[RACC_N], c_poly[RACC_N];
    uint8_t seed[RACC_AS_SZ];
    

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
            round_shift_r(vw[i], RACC_QW, RACC_NUW);
        }

        //  --- 10. c_hash := ChalHash(w, mu)
        xof_chal_hash(sig->ch, mu, vw);

        //  --- 11. c_poly := ChalPoly(c_hash)
        xof_chal_poly(c_poly, sig->ch);
        polyr_fntt(c_poly);

        for (i = 0; i < RACC_ELL; i++)
        {

            //  --- 12. [[s]] <- Refresh([[s]])
            racc_ntt_refresh(sk->s[i], &mrg);

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
            racc_ntt_refresh(mr[i], &mrg);

            //  --- 16. z := Decode([[z]])
            racc_ntt_decode(sig->z[i], mr[i]);

            //  Two consecutive multiplications: Montgomery adjustment
            polyr_ntt_smul(vz[i], sig->z[i], MONT_RRR1, MONT_RRR2);

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
            polyr_ntt_subq(y, y, u);
            polyr_intt(y);

            //  --- 18. h := w - round( y )_q->q_w
            round_shift_r(y, RACC_QW, RACC_NUW);
            polyr_subm(y, vw[i], y, RACC_QW);
            polyr_center(sig->h[i], y, RACC_QW);
        }

        //  --- 19. sig := (c_hash, h, z)                   [caller]

        //  --- 20. if CheckBounds(sig) = FAIL goto Line 4
        rsp = racc_check_bounds(sig->h, sig->z);

    } while (!rsp);

    //  --- 21. return sig                                  [caller]
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
    for (i = 0; i < RACC_K; i++) {
        for (j = 0; j < RACC_ELL; j++) {
            expand_aij(ma[i][j], i, j, sk->pk.a_seed);
        }
    }

    do {

        for (i = 0; i < RACC_ELL; i++) {

            //  --- 4.  [[r]] <- ZeroEncoding()
            zero_encoding(mr[i], &mrg);

            //  --- 5.  [[r]] <- AddRepNoise([[r]], uw, rep)
            add_rep_noise(mr[i], i, RACC_UW, &mrg);

            //  (Convert to NTT domain)
            for (j = 0; j < RACC_D; j++) {
                polyr_fntt(mr[i][j]);
            }
        }

        for (i = 0; i < RACC_K; i++) {

            //  --- 6.  [[w]] := A * [[r]]
            for (j = 0; j < RACC_D; j++) {
                polyr_ntt_cmul(mw[j], mr[0][j], ma[i][0]);
                for (k = 1; k < RACC_ELL; k++) {
                    polyr_ntt_mula(mw[j], mr[k][j], ma[i][k], mw[j]);
                }
                polyr_intt(mw[j]);
            }

            //  --- 7.  [[w]] <- AddRepNoise([[w]], uw, rep)
            add_rep_noise(mw, i, RACC_UW, &mrg);

            //  --- 8.  w := Decode([[w]])
            racc_decode(vw[i], mw);

            //  --- 9.  w := round( w )_q->q_w
            round_shift_r(vw[i], RACC_QW, RACC_NUW);
        }

        //  --- 10. c_hash := ChalHash(w, mu)
        xof_chal_hash(sig->ch, mu, vw);

        //  --- 11. c_poly := ChalPoly(c_hash)
        xof_chal_poly(c_poly, sig->ch);
        polyr_fntt(c_poly);

        for (i = 0; i < RACC_ELL; i++) {

            //  --- 12. [[s]] <- Refresh([[s]])
            racc_ntt_refresh(sk->s[i], &mrg);

            //  --- 13. [[r]] <- Refresh([[r]])
            racc_ntt_refresh(mr[i], &mrg);

            //  --- 14. [[z]] := c_poly * [[s]] + [[r]]
            for (j = 0; j < RACC_D; j++) {
                //  due to 2x Montgomery
                polyr_ntt_smul(u, mr[i][j], MONT_RI1, MONT_RI2);
                polyr_ntt_mula(mr[i][j], c_poly, sk->s[i][j], u);
            }

            //  --- 15. [[r]] <- Refresh([[r]])
            racc_ntt_refresh(mr[i], &mrg);

            //  --- 16. z := Decode([[z]])
            racc_ntt_decode(sig->z[i], mr[i]);

            //  Two consecutive multiplications: Montgomery adjustment
            polyr_ntt_smul(vz[i], sig->z[i], MONT_RRR1, MONT_RRR2);

            //  Decode for signature
            polyr_intt(sig->z[i]);
        }

        for (i = 0; i < RACC_K; i++) {

            //  --- 17. y := A*z - 2^{nu_t} * c_poly * t
            polyr_ntt_cmul(y, ma[i][0], vz[0]);
            for (j = 1; j < RACC_ELL; j++) {
                polyr_ntt_mula(y, ma[i][j], vz[j], y);
            }
            polyr_shlm(u, sk->pk.t[i], RACC_NUT, RACC_Q);

            polyr_fntt(u);
            polyr_ntt_cmul(u, u, c_poly);
            polyr_ntt_subq(y, y, u);
            polyr_intt(y);

            //  --- 18. h := w - round( y )_q->q_w
            round_shift_r(y, RACC_QW, RACC_NUW);
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
bool racc_core_verify(  const racc_sig_t *sig,
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
    if (!racc_check_bounds(sig->h, sig->z)) {
        return false;
    }
    //  --- 3.  mu := H( H(vk) || msg )                     [caller]

    //  --- 5.  c_poly := ChalPoly(c_hash)
    xof_chal_poly(c_poly, sig->ch);
    polyr_fntt(c_poly);

    for (i = 0; i < RACC_ELL; i++) {
        polyr_copy(vz[i], sig->z[i]);
        polyr_fntt(vz[i]);
    }

    for (i = 0; i < RACC_K; i++) {
        for (j = 0; j < RACC_ELL; j++) {

            //  --- 4.  A := ExpandA(seed)
            expand_aij(aij, i, j, pk->a_seed);

            //  --- 6.  y = A * z - 2^{nu_t} * c_poly * t
            if (j == 0) {
                polyr_ntt_cmul(t, aij, vz[0]);
            } else {
                polyr_ntt_mula(t, aij, vz[j], t);
            }
        }

        polyr_shlm(u, pk->t[i], RACC_NUT, RACC_Q);  //  .. - p_t * t ..
        polyr_fntt(u);
        polyr_ntt_cmul(u, u, c_poly);               //  .. Cpoly ..
        polyr_ntt_subq(vw[i], t, u);
        polyr_intt(vw[i]);

        //  --- 7.  w' = round( y )_q->q_w + h
        round_shift_r(vw[i], RACC_QW, RACC_NUW);
        polyr_nonneg(u, sig->h[i], RACC_QW);
        polyr_addm(vw[i], vw[i], u, RACC_QW);
    }

    //  --- 8. c_hash' := ChalHash(w', mu)
    xof_chal_hash(c_hchk, mu, vw);

    //  --- 9. if c_hash != c_hash' return FAIL
    //  --- 10. (else) return OK
    return ct_equal(c_hchk, sig->ch, RACC_CH_SZ);
}
