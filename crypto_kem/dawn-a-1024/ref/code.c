#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "param.h"

uint64_t pk_table[] = {
    1, 769, 591361, 454756609, 349707832321
};

uint64_t ct_table[] = {
    1, 193, 37249, 7189057, 1387488001
};

void encode_pk(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[205] = {0}, res[205], t = 0;

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            tmp[idx] += (uint64_t)(c[i + j] * pk_table[j]);
        }
        idx++;
    }
    idx = 0;

    for(i = 0; i < 4; i++)
    {
        tmp[204] += (uint64_t)(c[DIM_N - 4 + i] * pk_table[i]);
    }

    for(i = 0; i < 204; i += 4)
    {
        res[idx++] = tmp[i] + ((tmp[i + 3] & 65535) << 48);
        tmp[i + 3] = (tmp[i + 3] >> 16);
        res[idx++] = tmp[i + 1] + ((tmp[i + 3] & 65535) << 48);
        tmp[i + 3] = (tmp[i + 3] >> 16);
        res[idx++] = tmp[i + 2] + ((tmp[i + 3] & 65535) << 48);
    }

    res[idx++] = tmp[204];
    tmp_code = &res;
    memcpy(code_c, tmp_code, PKLEN * sizeof(uint8_t));

}

void decode_pk(uint8_t *code_c, int16_t *c)
{
    int i, j, idx = 153;
    uint64_t *res, tmp[205] = {0}, t = 0;

    res = code_c;

    tmp[204] = res[idx--] & 549755813887;

    for(i = 203; i > 0; i -= 4)
    {
        tmp[i - 1] = res[idx] & 281474976710655;
        tmp[i] = (((res[idx--] >> 48) & 65535) << 32);
        tmp[i - 2] = res[idx] & 281474976710655;
        tmp[i] += (((res[idx--] >> 48) & 65535) << 16);
        tmp[i - 3] = res[idx] & 281474976710655;
        tmp[i] += ((res[idx--] >> 48) & 65535);
    }

    for(i = 0; i < 4; i++)
    {
        c[DIM_N - 4 + i] = (uint16_t)(tmp[204] % 769);
        tmp[204] = tmp[204] / 769;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 5)
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
    uint64_t tmp[205] = {0}, res[103], t = 0;

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            tmp[idx] += (uint64_t)(c[i + j] * ct_table[j]);
        }
        idx++;
    }
    idx = 0;

    for(i = 0; i < 4; i++)
    {
        tmp[204] += (uint64_t)(c[DIM_N - 4 + i] * ct_table[i]);
    }

    for(i = 0; i < 204; i += 2)
    {
        res[idx++] = (tmp[i] & 67108863) + (tmp[i + 1] << 26);
    }

    res[idx] = tmp[204];
    
    tmp_code = &res;
    memcpy(code_c, tmp_code, 820 * sizeof(uint8_t));

    idx = 820;
    for(i = 0; i < 204; i += 4)
    {
        code_c[idx++] = (tmp[i] >> 26) & 255;
        code_c[idx++] = ((tmp[i] >> 34) << 4) + ((tmp[i + 2] >> 26) & 15);
        code_c[idx++] = tmp[i + 2] >> 30;
    }

}


void decode_ct(uint8_t *code_c, int16_t *c)
{
    int i, j, idx;
    uint64_t *res, tmp[205] = {0}, t;

    idx = 820;
    for(i = 0; i < 204; i += 4)
    {
        tmp[i] = code_c[idx++] + ((code_c[idx] >> 4) << 8);
        tmp[i] = tmp[i] << 26;
        tmp[i + 2] = (code_c[idx++] & 15) + (code_c[idx++] << 4);
        tmp[i + 2] = tmp[i + 2] << 26;
    }

    res = code_c;
    idx = 0;
    for(i = 0; i < 204; i += 2)
    {
        tmp[i] += res[idx] & 67108863;
        tmp[i + 1] = res[idx++] >> 26;

    }
    tmp[204] = res[102] & 2147483647;


    for(i = 0; i < 4; i++)
    {
        c[DIM_N - 4 + i] = (uint16_t)(tmp[204] % 193);
        tmp[204] = tmp[204] / 193;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 5)
    {
        for(j = 0; j < 5; j++)
        {
            c[i + j] = (uint16_t)(tmp[idx] % 193);
            tmp[idx] = tmp[idx] / 193;
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
