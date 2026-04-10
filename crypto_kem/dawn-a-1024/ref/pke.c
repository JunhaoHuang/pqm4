#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "param.h"
#include "sample.h"
#include "mq_ntt.h"
#include "poly.h"
#include "mq_ntt_param.h"
#include "code.h"

inline int16_t montgomery_reduce(int32_t a)
{
  int32_t t;
  int16_t u;

  u = a * QINV;
  t = (int32_t)u * Q;
  t = a - t;
  t >>= 16;
  return t;
}

int16_t FindOneIdx(int16_t *a, int16_t n, int16_t *idx)
{
    int i;
    int16_t ONENUM = 0;
    for(i = 0; i < n; i++)
    {
        if(a[i])
        {
            idx[ONENUM++] = i;
        }
    }
    return ONENUM;
}

void Mul_in_R2_n(int16_t *a, int16_t *b, int16_t n, int16_t *res)
{
    int16_t tmp_b[2 * n], idx[n];
    int16_t i, j, ONENUM;
    int16_t *v;

    for(i = 0; i < n; i++)
    {
        tmp_b[i] = b[i];
    }
    memcpy(tmp_b + n, b, n * sizeof(int16_t));
    ONENUM = FindOneIdx(a, n, idx);

    memset(res, 0, n * sizeof(int16_t));
    
    for(i = 0; i < ONENUM; i++)
    {
        v = tmp_b + n - idx[i];
        for(j = 0; j < n; j++)
        {
            res[j] ^= v[j];
        }
    }
}

void Mulf_in_R2_N2(int16_t *a, int16_t *b, int16_t *res)
{
    int16_t idx[N_2];
    int16_t i, j, ONENUM;
    int16_t *v;

    ONENUM = FindOneIdx(a, N_2, idx);

    memset(res, 0, N_2 * sizeof(int16_t));
    
    for(i = 0; i < ONENUM; i++)
    {
        v = b + N_2 - idx[i];
        for(j = 0; j < N_2; j++)
        {
            res[j] ^= v[j];
        }
    }
}

int FastInversion(int16_t *f, int16_t *f_inv)
{
    int16_t l, i, j, n;
    int16_t k[N_2] = {0}, b[N_2] = {0}, tmp_f[2 * N_2], tmp[N_2];

    for(i = 0; i < N_2; i++)
    {
        tmp_f[i] = f[i];
    }
    memcpy(tmp_f + N_2, tmp_f, N_2 * sizeof(int16_t));

    k[0] = f[0];
    for(i = 1; i < N_2; i++)
    {
        k[i] = f[i] ^ k[i - 1];
    }

    //level 0
    memset(f_inv, 0, N_2 * sizeof(int16_t));
    f_inv[0] = 1;
    for(i = 0; i < N_2; i++)
    {
        b[0] ^= k[i];
    }

    for(i = 0; i < N_2; i++)
    {
        k[i] ^= (b[0] * f[i]);
    }
    
    for(i = 1; i < N_2; i++)
    {
        k[i] = k[i] ^ k[i-1];
    }
    f_inv[0] = !b[0];
    f_inv[1] = b[0];

    //level 1 - l-1
    n = 1;
    for(l = 1; l < 8; l++)
    {
        n = 2 * n;
        memset(tmp, 0, n * sizeof(int16_t));
        for(i = 0; i < n; i++)
        {
            for(j = i; j < N_2; j += n)
            {
                tmp[i] ^= k[j];
            }
        }
        Mul_in_R2_n(f_inv, tmp, n, b);

        Mulf_in_R2_N2(b, tmp_f, tmp);

        for(j = 0; j < N_2; j++)
        {
            k[j] = k[j] ^ tmp[j];
        }

        for(i = n; i < N_2; i += n)
        {
            for(j = i; j < i + n; j++)
            {
                k[j] = k[j] ^ k[j - n];
            }
        }

        for(i = 0; i < n; i++)
        {
            tmp[i] = tmp[i + n] = b[i];
        }
        for(i = 0; i < 2 * n; i++)
        {
            f_inv[i] ^= tmp[i];
        }
    }

    return 0;
    
}

void PKE_KeyGen(uint8_t *seed, int16_t *pk, int16_t *f, uint8_t *f2, uint8_t *k)
{
    int16_t i, j, num;
    int16_t f_inv[DIM_N], g[DIM_N], temp[DIM_N], f_N[DIM_N];

    shake256ctx state;

    shake256_absorb(&state, seed, SEEDLEN);

    ternary_sample_fgk(f, g, k, &state);

    for(;;)
    {
        memcpy(f_N, f, DIM_N * sizeof(int16_t));
        for(i = 0; i < N_2; i++)
        {
            f_N[i] = (f_N[i] & 1) ^ (f_N[i + N_2] & 1) ^ (f_N[i + 2 * N_2] & 1) ^ (f_N[i + 3 * N_2] & 1);
        }
        if(check_poly_inv_Z2(f_N))
        {
            ternary_sample_f(f, &state);
            continue;
        }
        
        mq_poly_ntt_mq(f);
        if(check_poly_inv_Zq(f))
        {
            ternary_sample_f(f, &state);
            continue;
        }

        mq_poly_inv_ntt(f_inv, f);

        FastInversion(f_N, temp);
        for(i = 7; i >= 0; i--)
        {
            for(j = 0; j < SKF2LEN; j++)
            {
                f2[j] = (f2[j] << 1);
                f2[j] += temp[i * SKF2LEN + j];
            }
        }

        break;
    }
    for(;;)
    {
        mq_poly_ntt(g);
        if(check_poly_inv_Zq(g))
        {
            ternary_sample_g(g, &state);
            continue;
        }
        break;
    }

    mq_poly_pointwise_mul_mq(pk, g, f_inv);
}

void PKE_Encrypt(uint8_t *c, int16_t *pk, uint8_t *m, uint8_t *seed)
{
    int16_t i, j;
    int16_t s[DIM_N], e[DIM_N], tmp1[DIM_N], tmp2[DIM_N];

    shake256ctx state;
    shake256_absorb(&state, seed, SEEDLEN);
    ternary_sample_se(s, e, &state);

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < MESSLEN; j++)
        {
            tmp1[i * MESSLEN + j] = (m[j] & 1);
            m[j] = (m[j] >> 1);
        }
    }

    for(i = 0; i < DIM_N / 4; i++)
    {
        tmp1[i] = tmp1[i + 256] = tmp1[i + 512] = tmp1[i + 768] = tmp1[i] * 385;
    }

    mq_poly_ntt(s);
    mq_poly_pointwise_mul(tmp2, pk, s);
    mq_poly_intt(tmp2);


    for(i = 0; i < DIM_N; i++)
    {
        tmp1[i] = montgomery_reduce((tmp2[i] + e[i] + tmp1[i]) * 171);
        tmp1[i] += (tmp1[i] >> 15) & Q;
    }

    poly_round(tmp1);

    encode_ct(tmp1, c);
}

void PKE_Decrypt(uint8_t *c, int16_t *f, uint8_t *f2, uint8_t *m)
{
    int16_t i, j, idx, num0, num1, num2, num3, t1, t2, t3, t4;
    int16_t tmp1[DIM_N], tmp2[DIM_N], mp[DIM_N], cp[DIM_N], ep[DIM_N / 4], tmp_f2[N];

    decode_ct(c, tmp1);
    for(i = 0; i < DIM_N; i++)
    {
        tmp1[i] = tmp1[i] << 2;
    }

    mq_poly_ntt(tmp1);
    mq_poly_pointwise_mul(tmp2, tmp1, f);
    mq_poly_intt(tmp2);

    for(i = 0; i < N; i++)
    {
        cp[i] = montgomery_reduce((tmp2[i + N] - tmp2[i]) * 171);
    }

    for(i = N; i < DIM_N; i++)
    {
        cp[i] = montgomery_reduce((tmp2[i - N] + tmp2[i]) * 171);
    }

    for(i = 0; i < N; i++)
    {
        tmp2[i] = (cp[i] & 1) ^ (cp[i + N] & 1);
    }

    memset(tmp_f2 + N_2, 0, N_2 * sizeof(int16_t));
    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < SKF2LEN; j++)
        {
            tmp_f2[i * SKF2LEN + j] = (f2[j] & 1);
            f2[j] = (f2[j] >> 1);
        }
    }

    Mul_in_R2_n(tmp2, tmp_f2, N, mp);

    for(i = 0; i < DIM_N / 4; i++)
    {
        ep[i] = (cp[i] & 1) ^ (cp[i + 256] & 1) ^ (cp[i + 512] & 1) ^ (cp[i + 768] & 1);
    }

    idx = 0;
    for(i = 0; i < DIM_N / 4; i++)
    {
        idx = (ep[i] == 1) ? i : idx;
    }
    j = (idx == 0 && ep[0] == 0) ? 0 : 1;

    num0 = (cp[idx] >= 0) ? Q - cp[idx] : cp[idx] + Q;
    num1 = (cp[idx + 256] >= 0) ? Q - cp[idx + 256] : cp[idx + 256] + Q;
    num2 = (cp[idx + 512] >= 0) ? Q - cp[idx + 512] : cp[idx + 512] + Q;
    num3 = (cp[idx + 768] >= 0) ? Q - cp[idx + 768] : cp[idx + 768] + Q;

    t1 = (num0 <= num1) ? idx: idx + 256;
    t2 = (num2 <= num3) ? idx + 512: idx + 768;
    t3 = (num0 <= num1) ? num0: num1;
    t4 = (num2 <= num3) ? num2: num3;
    idx = (t3 <= t4) ? t1: t2;
    idx = (idx >= 512) ? idx - 512 : idx;

    for(i = 0; i < idx; i++)
    {
        tmp1[i] = f2[N - idx + i]; 
    }
    for(i = idx; i < N; i++)
    {
        tmp1[i] = f2[i - idx]; 
    }

    for(i = 0; i < DIM_N / 4; i++)
    {
        tmp2[i] = mp[i] ^ tmp1[i];
    }

    if(j)
    {
        for(i = 7; i >= 0; i--)
        {
            for(j = 0; j < MESSLEN; j++)
            {
                m[j] = (m[j] << 1);
                m[j] += tmp2[i * MESSLEN + j];
            }
        }
    }
    else
    {
        for(i = 7; i >= 0; i--)
        {
            for(j = 0; j < MESSLEN; j++)
            {
                m[j] = (m[j] << 1);
                m[j] += mp[i * MESSLEN + j];
            }
        }
    }
    
}