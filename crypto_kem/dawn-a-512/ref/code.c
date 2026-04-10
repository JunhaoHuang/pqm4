#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "param.h"

uint64_t pk_table[] = {
    1, 769, 591361, 454756609, 349707832321
};

uint64_t ct_table[] = {
    1, 111, 12321, 1367631, 151807041
};

void encode_pk(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[103] = {0}, res[103], t = 0;
    
    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            tmp[idx] += (uint64_t)(c[i + j] * pk_table[j]);
        }
        idx++;
    }
    idx = 0;

    for(i = 0; i < 2; i++)
    {
        tmp[102] += (uint64_t)(c[DIM_N - 2 + i] * pk_table[i]);
    }

    for(i = 0; i < 100; i += 4)
    {
        res[idx++] = tmp[i] + ((tmp[i + 3] & 65535) << 48);
        tmp[i + 3] = (tmp[i + 3] >> 16);
        res[idx++] = tmp[i + 1] + ((tmp[i + 3] & 65535) << 48);
        tmp[i + 3] = (tmp[i + 3] >> 16);
        res[idx++] = tmp[i + 2] + ((tmp[i + 3] & 65535) << 48);
    }

    res[idx++] = tmp[100] + ((tmp[102] & 65535) << 48);
    tmp[102] = (tmp[102] >> 16);
    res[idx++] = tmp[101] + ((tmp[102] & 15) << 48);

    tmp_code = &res;
    memcpy(code_c, tmp_code, PKLEN * sizeof(uint8_t));

}

void decode_pk(uint8_t *code_c, int16_t *c)
{
    int i, j, idx = 76;
    uint64_t *res, tmp[103] = {0}, t = 0;

    res = code_c;

    tmp[101] = res[idx] & 281474976710655;
    tmp[102] = (((res[idx--] >> 48) & 15) << 16);
    tmp[100] = res[idx] & 281474976710655;
    tmp[102] += ((res[idx--] >> 48) & 65535);

    for(i = 99; i > 0; i -= 4)
    {
        tmp[i - 1] = res[idx] & 281474976710655;
        tmp[i] = (((res[idx--] >> 48) & 65535) << 32);
        tmp[i - 2] = res[idx] & 281474976710655;
        tmp[i] += (((res[idx--] >> 48) & 65535) << 16);
        tmp[i - 3] = res[idx] & 281474976710655;
        tmp[i] += ((res[idx--] >> 48) & 65535);
    }

    for(i = 0; i < 2; i++)
    {
        c[DIM_N - 2 + i] = (uint16_t)(tmp[102] % 769);
        tmp[102] = tmp[102] / 769;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            c[i + j] = (uint16_t)(tmp[idx] % 769);
            tmp[idx] = tmp[idx] / 769;
        }
        idx++;
    }
}

void encode_ct(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[103] = {0}, res[103], t;
    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            tmp[idx] += (uint64_t)(c[i + j] * ct_table[j]);
        }
        idx++;
    }
    idx = 0;

    for(i = 0; i < 2; i++)
    {
        tmp[102] += (uint64_t)(c[DIM_N - 2 + i] * ct_table[i]);
    }

    t = 0;
    for(i = 0; i < 96; i += 32)
    {
        for(j = 0; j < 32; j += 2)
        {
            res[idx++] = tmp[i + j] + ((tmp[i + j + 1] & 1073741823) << 34);
            t = t << 4;
            t += (tmp[i + j + 1] >> 30);
        }  
        res[idx++] = t;
    }

    t=0;
    for(i = 96; i < 102; i += 2)
    {
        res[idx++] = tmp[i] + ((tmp[i + 1] & 1073741823) << 34);
        t = t << 4;
        t +=  (tmp[i + 1] >> 30);
    }
    t = (t << 14) + tmp[102];

    tmp_code = &res;
    for(i = 432; i < 436; i++)
    {
        tmp_code[i] = t & 255;
        t = t >> 8;
    }
    memcpy(code_c, tmp_code, 436 * sizeof(uint8_t));

}

void decode_ct(uint8_t *code_c, int16_t *c)
{
    int i, j, idx = 53;
    uint64_t *res, tmp[103] = {0}, t = 0, k;

    for(i = 435; i > 431; i--)
    {
        t = t << 8;
        t += code_c[i];
    }
    res = code_c;

    tmp[102] = t & 16383;
    t = t >> 14;

    for(i = 100; i > 95; i -= 2)
    {
        tmp[i] = (res[idx] & 17179869183);
        k = (res[idx--] >> 34);
        tmp[i + 1] = k + ((t & 15) << 30);
        t = t >> 4;
    }

    for(i = 64; i >= 0; i -= 32)
    {
        t = res[idx--];
        for(j = 30; j >= 0; j -= 2)
        {
            tmp[i + j] = (res[idx] & 17179869183);
            k = (res[idx--] >> 34);
            tmp[i + j + 1] = k + ((t & 15) << 30);
            t = t >> 4;
        }
    }

    for(i = 0; i < 2; i++)
    {
        c[DIM_N - 2 + i] = (uint16_t)(tmp[102] % 111);
        tmp[102] = tmp[102] / 111;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            c[i + j] = (uint16_t)(tmp[idx] % 111);
            tmp[idx] = tmp[idx] / 111;
        }
        idx++;
    }

}

void encode_f(int16_t *c, uint8_t *code_c)
{
    int i, j, t;

    j = 0;
    for(i = 0; i < DIM_N; i += 64)
    {
        for(t = 0; t < 64; t++)
        {
            code_c[j + t] = c[i + t] & 255;
        }
        j += 64;
        for(t = 0; t < 16; t++)
        {
            code_c[j + t] = (c[i + 48 + t] >> 8) | ((c[i + 32 + t] >> 8) << 2) | ((c[i + 16 + t] >> 8) << 4) | ((c[i + t] >> 8) << 6);
        }
        j += 16;
    }
}

void decode_f(uint8_t *code_c, int16_t *c)
{
    int i, j, t;

    j = 0;
    for(i = 0; i < DIM_N; i += 64)
    {
        for(t = 0; t < 16; t++)
        {
            c[i + 48 + t] = ((code_c[j + 64 + t] & 3) << 8);
            c[i + 32 + t] = (((code_c[j + 64 + t] >> 2) & 3) << 8);
            c[i + 16 + t] = (((code_c[j + 64 + t] >> 4) & 3) << 8);
            c[i + t] = ((code_c[j + 64 + t] >> 6) << 8);
        }

        for(t = 0; t < 64; t++)
        {
            c[i + t] += code_c[j + t];
        }
        j += 80;
    }
}