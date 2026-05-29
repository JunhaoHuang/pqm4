#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "param.h"
#include "sample.h"
#include "poly.h"
#include "code.h"

void Mul_in_R2_n(int16_t *a, int16_t *b, int16_t n, int16_t *res)
{
    int16_t tmp_b[2 * n];
    memcpy(tmp_b, b, n * sizeof(int16_t));
    memcpy(tmp_b + n, b, n * sizeof(int16_t));
    memset(res, 0, n * sizeof(int16_t));
    Mul_in_R2_n_asm(res,a,tmp_b,n);
}

void Mul_in_R2_n2(int16_t *a, int16_t *b, int16_t n, int16_t *res)
{
    int16_t tmp_b[2 * n];
    memcpy(tmp_b, b, n * sizeof(int16_t));
    memcpy(tmp_b + n, b, n * sizeof(int16_t));

    memset(res, 0, n * sizeof(int16_t));
    Mul_in_R2_2_asm(res, a, tmp_b, n);
}

void Mulf_in_R2_N2(int16_t *a, int16_t *b, int16_t *res)
{
    memset(res, 0, N_2 * sizeof(int16_t));
    Mul_in_R2_n_asm(res, a, b, N_2);
}

int FastInversion(int16_t *f, int16_t *f_inv)
{
    int16_t l, i, j, n;
    int16_t k[N_2] = {0}, b[N_2] = {0}, tmp_f[2 * N_2], tmp[N_2];


    memcpy(tmp_f, f, N_2 * sizeof(int16_t));
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
    for(l = 1; l < 7; l++)
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
        if(n==2){
            Mul_in_R2_n2(f_inv, tmp, n, b);
        }
        else
        {
            Mul_in_R2_n(f_inv, tmp, n, b);
        }
        
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

        poly_xor4(f_N, f_N);
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

        poly_pack_f2(f2, temp);

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

    poly_unpack_f2(tmp1, m);

    poly_mul385(tmp1);

    mq_poly_ntt(s);
    mq_poly_pointwise_mul(tmp2, pk, s);
    mq_poly_intt(tmp2);

    poly_add(tmp1, tmp1, tmp2);
    poly_add(tmp1, tmp1, e);
    mq_poly_reduce_mq(tmp1, DIM_N);

    poly_round(tmp1);
    encode_ct(tmp1, c);

}

void PKE_Decrypt(uint8_t *c, int16_t *f, uint8_t *f2, uint8_t *m)
{
    int16_t i, j, idx, num0, num1, num2, num3, t1, t2, t3, t4;
    int16_t tmp1[DIM_N], tmp2[DIM_N], mp[DIM_N], cp[DIM_N], ep[N_2], tmp_f2[N];

    decode_ct(c, tmp1);

    poly_unround(tmp1);

    mq_poly_ntt(tmp1);
    mq_poly_pointwise_mul(tmp2, tmp1, f);
    mq_poly_intt(tmp2);

    poly_cp(cp, tmp2);

    memset(tmp_f2 + N_2, 0, N_2 * sizeof(int16_t));

    poly_unpack_f2(tmp_f2, f2);

    Mul_in_R2_n(tmp2, tmp_f2, N, mp);

    poly_xor4(ep, cp);

    idx = 0;
    for(i = 0; i < N_2; i++)
    {
        idx = (ep[i] == 1) ? i : idx;
    }
    j = (idx == 0 && ep[0] == 0) ? 0 : 1;

    num0 = (cp[idx] >= 0) ? Q - cp[idx] : cp[idx] + Q;
    num1 = (cp[idx + 128] >= 0) ? Q - cp[idx + 128] : cp[idx + 128] + Q;
    num2 = (cp[idx + 256] >= 0) ? Q - cp[idx + 256] : cp[idx + 256] + Q;
    num3 = (cp[idx + 384] >= 0) ? Q - cp[idx + 384] : cp[idx + 384] + Q;

    t1 = (num0 <= num1) ? idx: idx + 128;
    t2 = (num2 <= num3) ? idx + 256: idx + 384;
    t3 = (num0 <= num1) ? num0: num1;
    t4 = (num2 <= num3) ? num2: num3;
    idx = (t3 <= t4) ? t1: t2;
    idx = (idx >= 256) ? idx - 256 : idx;

    for(i = 0; i < idx; i++)
    {
        tmp1[i] = tmp_f2[N - idx + i]; 
    }
    for(i = idx; i < N; i++)
    {
        tmp1[i] = tmp_f2[i - idx]; 
    }

    for(i = 0; i < N_2; i++)
    {
        tmp2[i] = mp[i] ^ tmp1[i];
    }

    if(j)
    {
        poly_pack_f2(m, tmp2);
    }
    else
    {
        poly_pack_f2(m, mp);
    }
    
}
