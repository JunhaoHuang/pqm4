#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "param.h"

uint64_t pkct_table1[] = {
    1, 86, 7396, 636056, 54700816, 4704270176, 404567235136
};

uint32_t pkct_table2[] = {
    1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721
};


void encode_pk(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[147] = {0}, res[147];
    uint32_t tmp_32[61] = {0}, res_32[50];
    int16_t tmp_c[DIM_N];

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 86;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp[idx] += (uint64_t)(tmp_c[i + j] * pkct_table1[j]);
        }
        idx++;
    }
    idx = 0;

    for(i = 0; i < 2; i++)
    {
        tmp[146] += (uint64_t)(tmp_c[DIM_N - 2 + i] * pkct_table1[i]);
    }

    j = 0;
    for(i = 0; i < 144; i += 3)
    {
        res[j] = (tmp[i + 1] << 19) + (tmp[i] & 524287);
        res[j + 1] = (tmp[i + 2] << 19) + ((tmp[i] >> 19) & 524287);
        j += 2;
    }

    tmp_code = (uint8_t*)&res;
    memcpy(code_c, tmp_code, 768 * sizeof(uint8_t));
    
    idx = 768;
    for(i = 0; i < 144; i += 24)
    {
        code_c[idx] = ((tmp[i] >> 38) << 1) + ((tmp[i + 21] >> 38) & 1);
        code_c[idx + 1] = ((tmp[i + 3] >> 38) << 1) + ((tmp[i + 21] >> 39) & 1);
        code_c[idx + 2] = ((tmp[i + 6] >> 38) << 1) + ((tmp[i + 21] >> 40) & 1);
        code_c[idx + 3] = ((tmp[i + 9] >> 38) << 1) + ((tmp[i + 21] >> 41) & 1);
        code_c[idx + 4] = ((tmp[i + 12] >> 38) << 1) + ((tmp[i + 21] >> 42) & 1);
        code_c[idx + 5] = ((tmp[i + 15] >> 38) << 1) + ((tmp[i + 21] >> 43) & 1);
        code_c[idx + 6] = ((tmp[i + 18] >> 38) << 1) + ((tmp[i + 21] >> 44) & 1);
        idx += 7;
    }

    for(i = 0; i < 5; i++)
    {
        code_c[idx++] = (tmp[144] >> (i * 8)) & 255;
    }
    for(i = 0; i < 5; i++)
    {
        code_c[idx++] = (tmp[145] >> (i * 8)) & 255;
    }

    code_c[idx++] = (((tmp[144] >> 40) & 31) << 3) + (tmp[146] & 7);
    code_c[idx++] = (((tmp[145] >> 40) & 31) << 3) + ((tmp[146] >> 3) & 7);
    code_c[idx++] = (tmp[146] >> 6);

    memset(tmp_32, 0, 31 * sizeof(uint32_t));

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 17)
    {
        for(j = 0; j < 17; j++)
        {
            tmp_32[idx] += (uint32_t)(tmp_c[i + j] * pkct_table2[j]);
        }
        idx++;
    }

    for(i = 0; i < 4; i++)
    {
        tmp_32[60] += (uint32_t)(tmp_c[DIM_N - 4 + i] * pkct_table2[i]);
    }

    j = 0;
    for(i = 0; i <60; i += 6)
    {
        res_32[j] = tmp_32[i] | ((tmp_32[i + 5] & 31) << 27);
        res_32[j + 1] = tmp_32[i + 1] | (((tmp_32[i + 5] >> 5) & 31) << 27);
        res_32[j + 2] = tmp_32[i + 2] | (((tmp_32[i + 5] >> 10) & 31) << 27);
        res_32[j + 3] = tmp_32[i + 3] | (((tmp_32[i + 5] >> 15) & 31) << 27);
        res_32[j + 4] = tmp_32[i + 4] | (((tmp_32[i + 5] >> 20) & 31) << 27);
        j += 5;
        tmp_32[60] = tmp_32[60] << 2;
        tmp_32[60] += (tmp_32[i + 5] >> 25);
    }
    
    tmp_code = (uint8_t*)&res_32;
    memcpy(code_c + 823, tmp_code, 200 * sizeof(uint8_t));

    code_c[1023] = tmp_32[60] & 255;
    code_c[1024] = (tmp_32[60] >> 8) & 255;
    code_c[1025] = (tmp_32[60] >> 16) & 255;
    code_c[1026] = (tmp_32[60] >> 24);
}

void decode_pk(uint8_t *code_c, int16_t *c)
{
    int i, j, idx;
    uint64_t *res, tmp[147] = {0};
    uint32_t *res_32, tmp_32[61] = {0};
    int16_t tmp_c1[DIM_N], tmp_c2[DIM_N];

    tmp[146] = (code_c[822] << 6) + ((code_c[821] & 7) << 3) + (code_c[820] & 7);
    tmp[145] = (code_c[821] >> 3);
    tmp[144] = (code_c[820] >> 3);


    idx = 819;
    for(i = 0; i < 5; i++)
    {
        tmp[145] = tmp[145] << 8;
        tmp[145] += code_c[idx--];
    }
    for(i = 0; i < 5; i++)
    {
        tmp[144] = tmp[144] << 8;
        tmp[144] += code_c[idx--];
    }

    idx = 768;
    for(i = 0; i < 144; i += 24)
    {
        tmp[i] = code_c[idx] >> 1;
        tmp[i] = tmp[i] << 38;
        tmp[i + 3] = code_c[idx + 1] >> 1;
        tmp[i + 3] = tmp[i + 3] << 38;
        tmp[i + 6] = code_c[idx + 2] >> 1;
        tmp[i + 6] = tmp[i + 6] << 38;
        tmp[i + 9] = code_c[idx + 3] >> 1;
        tmp[i + 9] = tmp[i + 9] << 38;
        tmp[i + 12] = code_c[idx + 4] >> 1;
        tmp[i + 12] = tmp[i + 12] << 38;
        tmp[i + 15] = code_c[idx + 5] >> 1;
        tmp[i + 15] = tmp[i + 15] << 38;
        tmp[i + 18] = code_c[idx + 6] >> 1;
        tmp[i + 18] = tmp[i + 18] << 38;
        tmp[i + 21] = (code_c[idx] & 1) | ((code_c[idx + 1] & 1) << 1) | ((code_c[idx + 2] & 1) << 2) | ((code_c[idx + 3] & 1) << 3) | ((code_c[idx + 4] & 1) << 4) | ((code_c[idx + 5] & 1) << 5) | ((code_c[idx + 6] & 1) << 6);
        tmp[i + 21] = tmp[i + 21] << 38;
        idx += 7;
    }

    res = (uint64_t*)code_c;
    j = 0;
    for(i = 0; i < 144; i += 3)
    {
        tmp[i] += (((res[j + 1] & 524287) << 19) + (res[j] & 524287));
        tmp[i + 1] = res[j] >> 19;
        tmp[i + 2] = res[j + 1] >> 19;
        j += 2;
    }

    for(i = 0; i < 2; i++)
    {
        tmp_c1[DIM_N - 2 + i] = (uint16_t)(tmp[146] % 86);
        tmp[146] = tmp[146] / 86;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp_c1[i + j] = (uint16_t)(tmp[idx] % 86);
            tmp[idx] = tmp[idx] / 86;
        }
        idx++;
    }

    tmp_32[60] = code_c[1023] | (code_c[1024] << 8) | (code_c[1025] << 16) | (code_c[1026] << 24);

    res_32 = (uint32_t*)(code_c + 823);

    j = 0;
    for(i = 0; i < 60; i += 6)
    {
        tmp_32[i] = res_32[j] & 134217727;
        tmp_32[i + 1] = res_32[j + 1] & 134217727;
        tmp_32[i + 2] = res_32[j + 2] & 134217727;
        tmp_32[i + 3] = res_32[j + 3] & 134217727;
        tmp_32[i + 4] = res_32[j + 4] & 134217727;
        tmp_32[i + 5] = ((res_32[j] >> 27) | ((res_32[j + 1] >> 27) << 5) | ((res_32[j + 2] >> 27) << 10) | ((res_32[j + 3] >> 27) << 15) | ((res_32[j + 4] >> 27) << 20));
        j += 5;
    }

    tmp_32[5] += (((tmp_32[60] >> 18) & 3) << 25);
    tmp_32[11] += (((tmp_32[60] >> 16) & 3) << 25);
    tmp_32[17] += (((tmp_32[60] >> 14) & 3) << 25);
    tmp_32[23] += (((tmp_32[60] >> 12) & 3) << 25);
    tmp_32[29] += (((tmp_32[60] >> 10) & 3) << 25);
    tmp_32[35] += (((tmp_32[60] >> 8) & 3) << 25);
    tmp_32[41] += (((tmp_32[60] >> 6) & 3) << 25);
    tmp_32[47] += (((tmp_32[60] >> 4) & 3) << 25);
    tmp_32[53] += (((tmp_32[60] >> 2) & 3) << 25);
    tmp_32[59] += ((tmp_32[60] & 3) << 25);
    tmp_32[60] >>= 20;

    for(i = 0; i < 4; i++)
    {
        tmp_c2[DIM_N - 4 + i] = (uint16_t)(tmp_32[60] % 3);
        tmp_32[60] = tmp_32[60] / 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 4; i += 17)
    {
        for(j = 0; j < 17; j++)
        {
            tmp_c2[i + j] = (uint16_t)(tmp_32[idx] % 3);
            tmp_32[idx] = tmp_32[idx] / 3;
        }
        idx++;
    }

    for(i = 0; i < DIM_N; i++)
    {
        c[i] = (tmp_c1[i] * 3 * 29 + tmp_c2[i] * 86 * 2) % 258;
    }

}

void encode_f(int16_t *c, uint8_t *code_c)
{
    int i, j, t;

    j = 0;
    for(i = 0; i < DIM_N; i += 128)
    {
        for(t = 0; t < 128; t++)
        {
            code_c[j + t] = c[i + t] & 255;
        }
        j += 128;
        for(t = 0; t < 16; t++)
        {
            code_c[j + t] = (c[i + 112 + t] >> 8) | ((c[i + 96 + t] >> 8) << 1) | ((c[i + 80 + t] >> 8) << 2) | ((c[i + 64 + t] >> 8) << 3) | ((c[i + 48 + t] >> 8) << 4) | ((c[i + 32 + t] >> 8) << 5) | ((c[i + 16 + t] >> 8) << 6) | ((c[i + t] >> 8) << 7);
        }
        j += 16;
    }
}

void decode_f(uint8_t *code_c, int16_t *c)
{
    int i, j, t;

    j = 0;
    for(i = 0; i < DIM_N; i += 128)
    {
        for(t = 0; t < 16; t++)
        {
            c[i + 112 + t] = ((code_c[j + 128 + t] & 1) << 8);
            c[i + 96 + t] = (((code_c[j + 128 + t] >> 1) & 1) << 8);
            c[i + 80 + t] = (((code_c[j + 128 + t] >> 2) & 1) << 8);
            c[i + 64 + t] = (((code_c[j + 128 + t] >> 3) & 1) << 8);
            c[i + 48 + t] = (((code_c[j + 128 + t] >> 4) & 1) << 8);
            c[i + 32 + t] = (((code_c[j + 128 + t] >> 5) & 1) << 8);
            c[i + 16 + t] = (((code_c[j + 128 + t] >> 6) & 1) << 8);
            c[i + t] = ((code_c[j + 128 + t] >> 7) << 8);
        }

        for(t = 0; t < 128; t++)
        {
            c[i + t] += code_c[j + t];
        }
        j += 144;
    }
}
