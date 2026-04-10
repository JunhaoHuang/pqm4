#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "param.h"
#include "fips202.h"

void ternary_sample_fgk(int16_t *f, int16_t *g, uint8_t *k, shake256ctx *state)
{
    uint8_t x, y;
    int16_t i, j, t;
    uint8_t buf[544]; // f 192 g 256
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];

    shake256_squeezeblocks(buf, 4, state);
    // shake256_squeeze(buf + 408, 72, state);
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
        tmp1[i] = tmp1[i] - tmp1[i + DIM_N];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN; j++)
        {
            tmp1[DIM_N + i * DIMLEN + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }
    t += DIMLEN;

    for(i = 0; i < DIM_N; i++)
    {
        f[i] = tmp1[i] * tmp1[i + DIM_N];
    }

	memset(g, 0, DIM_N * sizeof(int16_t));
	for (i = 0; i < DIM_N;) 
    {
		x = buf[t++];
		y = x & 15;
        if(y < 5)
            g[i] = -1;
        else if(y > 10)
            g[i] = 1;
        i++;

        y = (x >> 4);
        if(y < 5)
            g[i] = -1;
        else if(y > 10)
            g[i] = 1;
        i++;
	}
}

void ternary_sample_f(int16_t *f, shake256ctx *state)
{
    int16_t i, j, t;
    uint8_t buf[272];
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];

    shake256_squeezeblocks(buf, 2, state);
    // shake256_squeeze(buf + 136, 56, state);

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
        tmp1[i] = tmp1[i] - tmp1[i + DIM_N];
    }

    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < DIMLEN; j++)
        {
            tmp1[DIM_N + i * DIMLEN + j] = (buf[t + j] & 1);
            buf[t + j] = (buf[t + j] >> 1);
        }
    }

    for(i = 0; i < DIM_N; i++)
    {
        f[i] = tmp1[i] * tmp1[i + DIM_N];
    }
}

void ternary_sample_g(int16_t *g, shake256ctx *state)
{
	uint8_t x, y;
    int16_t i, j, t;
    uint8_t buf[272];
    shake256_squeezeblocks(buf, 2, state);

    t = 0;
    memset(g, 0, DIM_N * sizeof(int16_t));
	for (i = 0; i < DIM_N;) 
    {
		x = buf[t++];
		y = x & 15;
        if(y < 5)
            g[i] = -1;
        else if(y > 10)
            g[i] = 1;
        i++;

        y = (x >> 4);
        if(y < 5)
            g[i] = -1;
        else if(y > 10)
            g[i] = 1;
        i++;
	}
}

void ternary_sample_se(int16_t *s, int16_t *e, shake256ctx *state)
{
	uint8_t x, y;
    int16_t i, j, t;

    uint8_t buf[544];
    int8_t tmp1[DIM_N * 2], tmp2[DIM_N * 2];
    shake256_squeezeblocks(buf, 4, state);
    
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

    memset(e, 0, DIM_N * sizeof(int16_t));
	for (i = 0; i < DIM_N;) 
    {
		x = buf[t++];
		y = x & 15;
        if(y < 5)
            e[i] = -1;
        else if(y > 10)
            e[i] = 1;
        i++;

        y = (x >> 4);
        if(y < 5)
            e[i] = -1;
        else if(y > 10)
            e[i] = 1;
        i++;
	}

}