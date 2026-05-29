#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "param.h"
#include "fips202.h"

void ternary_sample_fgk(int16_t *f, int16_t *g, uint8_t *k, shake256ctx *state)
{
    //f 640 g 256
    int16_t i, j, t;
    uint8_t buf[7*SHAKE256_RATE];
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];

    shake256_squeezeblocks(buf, 7, state);
    memcpy(k, buf, SYMBYTES * sizeof(uint8_t));

    t = SYMBYTES;
	for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp1[i] = tmp1[i] & tmp1[i + DIM_N];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp2[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp2[i] = tmp2[i] & tmp2[i + DIM_N];
        tmp1[i] = tmp1[i] - tmp2[i];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN; j++)
        {
            tmp2[i * DIMLEN + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN;

    for(i = 0; i < DIM_N; i++)
    {
        f[i] = tmp1[i] * tmp2[i];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    for(i = 0; i < DIM_N; i++)
    {
        g[i] = tmp1[i] - tmp1[i + DIM_N];
    }
}

void ternary_sample_f(int16_t *f, shake256ctx *state)
{
    int16_t i, j, t;
    uint8_t buf[5*SHAKE256_RATE];
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];

    shake256_squeezeblocks(buf, 5, state);

    t = 0;
    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp1[i] = tmp1[i] & tmp1[i + DIM_N];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp2[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp2[i] = tmp2[i] & tmp2[i + DIM_N];
        tmp1[i] = tmp1[i] - tmp2[i];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN; j++)
        {
            tmp2[i * DIMLEN + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }

    for(i = 0; i < DIM_N; i++)
    {
        f[i] = tmp1[i] * tmp2[i];
    }
}

void ternary_sample_g(int16_t *g, shake256ctx *state)
{
    int16_t i, j, t;
    uint8_t buf[272];
    int8_t tmp1[DIM_N * 2];
    shake256_squeezeblocks(buf, 1, state);

    t = 0;
    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    for(i = 0; i < DIM_N; i++)
    {
        g[i] = tmp1[i] - tmp1[i + DIM_N];
    }
}

void ternary_sample_se(int16_t *s, int16_t *e, shake256ctx *state)
{
    //s 512 e 256
    int16_t i, j, t;
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];
    uint8_t buf[816];
    shake256_squeezeblocks(buf, 6, state);

	t = 0;
    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp1[i] = tmp1[i] & tmp1[i + DIM_N];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp2[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN * 2;

    for(i = 0; i < DIM_N; i++)
    {
        tmp2[i] = tmp2[i] & tmp2[i + DIM_N];
    }

    for(i = 0; i < DIM_N; i++)
    {
        s[i] = tmp1[i] - tmp2[i];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN * 2; j++)
        {
            tmp1[i * (DIMLEN * 2) + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    for(i = 0; i < DIM_N; i++)
    {
        e[i] = tmp1[i] - tmp1[i + DIM_N];
    }
}