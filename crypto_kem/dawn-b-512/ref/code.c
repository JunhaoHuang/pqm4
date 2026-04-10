#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "param.h"

uint64_t pk_table1[] = {
    1, 86, 7396, 636056, 54700816, 4704270176, 404567235136
};

uint32_t pk_table2[] = {
    1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721
};

uint64_t ct_table1[] = {
    1, 43, 1849, 79507, 3418801, 147008443, 6321363049
};

uint32_t ct_table2[] = {
    1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721
};

void encode_pk(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[74] = {0}, res[48];
    uint32_t tmp_32[31] = {0}, res_32[25], t;
    int16_t tmp_c[DIM_N];

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 86;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 1; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp[idx] += (uint64_t)(tmp_c[i + j] * pk_table1[j]);
        }
        idx++;
    }
    idx = 0;

    j = 0;
    for(i = 0; i < 72; i += 3)
    {
        res[j] = (tmp[i + 1] << 19) + (tmp[i] & 524287);
        res[j + 1] = (tmp[i + 2] << 19) + ((tmp[i] >> 19) & 524287);
        j += 2;
    }

    tmp_code = &res;
    memcpy(code_c, tmp_code, 384 * sizeof(uint8_t));
    
    idx = 384;
    for(i = 0; i < 72; i += 24)
    {
        code_c[idx] = ((tmp[i] >> 38) << 1) | ((tmp[i + 21] >> 38) & 1);
        code_c[idx + 1] = ((tmp[i + 3] >> 38) << 1) | ((tmp[i + 21] >> 39) & 1);
        code_c[idx + 2] = ((tmp[i + 6] >> 38) << 1) | ((tmp[i + 21] >> 40) & 1);
        code_c[idx + 3] = ((tmp[i + 9] >> 38) << 1) | ((tmp[i + 21] >> 41) & 1);
        code_c[idx + 4] = ((tmp[i + 12] >> 38) << 1) | ((tmp[i + 21] >> 42) & 1);
        code_c[idx + 5] = ((tmp[i + 15] >> 38) << 1) | ((tmp[i + 21] >> 43) & 1);
        code_c[idx + 6] = ((tmp[i + 18] >> 38) << 1) | ((tmp[i + 21] >> 44) & 1);
        idx += 7;
    }

    for(i = 0; i < 6; i++)
    {
        code_c[idx++] = (tmp[72] >> (i * 8)) & 255;
    }
    code_c[idx++] = tmp_c[DIM_N - 1];

    memset(tmp_32, 0, 31 * sizeof(uint32_t));

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 17)
    {
        for(j = 0; j < 17; j++)
        {
            tmp_32[idx] += (uint32_t)(tmp_c[i + j] * pk_table2[j]);
        }
        idx++;
    }

    for(i = 0; i < 2; i++)
    {
        tmp_32[30] += (uint32_t)(tmp_c[DIM_N - 2 + i] * pk_table2[i]);
    }

    j = 0;
    t = 0;
    for(i = 0; i < 30; i += 6)
    {
        res_32[j] = tmp_32[i] | ((tmp_32[i + 5] & 31) << 27);
        res_32[j + 1] = tmp_32[i + 1] | (((tmp_32[i + 5] >> 5) & 31) << 27);
        res_32[j + 2] = tmp_32[i + 2] | (((tmp_32[i + 5] >> 10) & 31) << 27);
        res_32[j + 3] = tmp_32[i + 3] | (((tmp_32[i + 5] >> 15) & 31) << 27);
        res_32[j + 4] = tmp_32[i + 4] | (((tmp_32[i + 5] >> 20) & 31) << 27);
        j += 5;
        t = t << 2;
        t += (tmp_32[i + 5] >> 25);
    }
    
    tmp_code = &res_32;
    memcpy(code_c + 412, tmp_code, 100 * sizeof(uint8_t));

    code_c[512] = t & 255;
    code_c[513] = (t >> 8) | (tmp_32[30] << 2);

}

void decode_pk(uint8_t *code_c, int16_t *c)
{
    int i, j, idx = 411;
    uint64_t *res, tmp[74] = {0};
    uint32_t *res_32, tmp_32[31] = {0}, t;
    int16_t tmp_c1[DIM_N], tmp_c2[DIM_N];

    tmp_c1[DIM_N - 1] = code_c[411];
    idx = 410;
    for(i = 0; i < 6; i++)
    {
        tmp[72] = tmp[72] << 8;
        tmp[72] += code_c[idx--];
    }

    idx = 384;
    for(i = 0; i < 72; i += 24)
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

    res = code_c;
    j = 0;
    for(i = 0; i < 72; i += 3)
    {
        tmp[i] += (((res[j + 1] & 524287) << 19) | (res[j] & 524287));
        tmp[i + 1] = res[j] >> 19;
        tmp[i + 2] = res[j + 1] >> 19;
        j += 2;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 1; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp_c1[i + j] = (uint16_t)(tmp[idx] % 86);
            tmp[idx] = tmp[idx] / 86;
        }
        idx++;
    }

    tmp_32[30] = code_c[513] >> 2;
    t = ((code_c[513] & 3) << 8) + code_c[512];

    res_32 = code_c + 412;

    j = 0;
    for(i = 0; i < 30; i += 6)
    {
        tmp_32[i] = res_32[j] & 134217727;
        tmp_32[i + 1] = res_32[j + 1] & 134217727;
        tmp_32[i + 2] = res_32[j + 2] & 134217727;
        tmp_32[i + 3] = res_32[j + 3] & 134217727;
        tmp_32[i + 4] = res_32[j + 4] & 134217727;
        tmp_32[i + 5] = ((res_32[j] >> 27) | ((res_32[j + 1] >> 27) << 5) | ((res_32[j + 2] >> 27) << 10) | ((res_32[j + 3] >> 27) << 15) | ((res_32[j + 4] >> 27) << 20));
        j += 5;
    }

    tmp_32[5] += ((t >> 8) << 25);
    tmp_32[11] += (((t >> 6) & 3) << 25);
    tmp_32[17] += (((t >> 4) & 3) << 25);
    tmp_32[23] += (((t >> 2) & 3) << 25);
    tmp_32[29] += ((t & 3) << 25);

    for(i = 0; i < 2; i++)
    {
        tmp_c2[DIM_N - 2 + i] = (uint16_t)(tmp_32[30] % 3);
        tmp_32[30] = tmp_32[30] / 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 17)
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

void encode_ct(int16_t *c, uint8_t *code_c)
{
    int i, j, idx;
    uint8_t *tmp_code;
    uint64_t tmp[74] = {0}, res[74];
    uint32_t tmp_32[31] = {0}, res_32[25], t;
    int16_t tmp_c[DIM_N];

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 43;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 1; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp[idx] += (uint64_t)(tmp_c[i + j] * ct_table1[j]);
        }
        idx++;
    }
    idx = 0;

    j = 0;
    for(i = 0; i < 72; i += 2)
    {
        res[j++] = (tmp[i + 1] << 26) + (tmp[i] & 67108863);
    }

    tmp_code = &res;
    memcpy(code_c, tmp_code, 288 * sizeof(uint8_t));

    idx = 288;
    for(i = 0; i < 72; i += 4)
    {
        code_c[idx] = (tmp[i] >> 26) & 255;
        code_c[idx + 1] = (tmp[i + 2] >> 26) & 255;
        code_c[idx + 2] = (tmp[i] >> 34) + ((tmp[i + 2] >> 34) << 4);
        idx += 3;
    }
    for(i = 0; i < 5; i++)
    {
        code_c[idx++] = (tmp[72] >> (i * 8)) & 255;
    }
    code_c[idx++] = tmp_c[DIM_N - 1];

    memset(tmp_32, 0, 31 * sizeof(uint32_t));

    for(i = 0; i < DIM_N; i++)
    {
        tmp_c[i] = c[i] % 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 17)
    {
        for(j = 0; j < 17; j++)
        {
            tmp_32[idx] += (uint32_t)(tmp_c[i + j] * pk_table2[j]);
        }
        idx++;
    }

    for(i = 0; i < 2; i++)
    {
        tmp_32[30] += (uint32_t)(tmp_c[DIM_N - 2 + i] * pk_table2[i]);
    }

    j = 0;
    t = 0;
    for(i = 0; i < 30; i += 6)
    {
        res_32[j] = tmp_32[i] | ((tmp_32[i + 5] & 31) << 27);
        res_32[j + 1] = tmp_32[i + 1] | (((tmp_32[i + 5] >> 5) & 31) << 27);
        res_32[j + 2] = tmp_32[i + 2] | (((tmp_32[i + 5] >> 10) & 31) << 27);
        res_32[j + 3] = tmp_32[i + 3] | (((tmp_32[i + 5] >> 15) & 31) << 27);
        res_32[j + 4] = tmp_32[i + 4] | (((tmp_32[i + 5] >> 20) & 31) << 27);
        j += 5;
        t = t << 2;
        t += (tmp_32[i + 5] >> 25);
    }
    
    tmp_code = &res_32;
    memcpy(code_c + 348, tmp_code, 100 * sizeof(uint8_t));

    code_c[448] = t & 255;
    code_c[449] = (t >> 8) | (tmp_32[30] << 2);
}   


void decode_ct(uint8_t *code_c, int16_t *c)
{
    int i, j, idx = 347;
    uint64_t *res, tmp[74] = {0};
    uint32_t *res_32, tmp_32[31] = {0}, t;
    int16_t tmp_c1[DIM_N], tmp_c2[DIM_N];


    tmp_c1[DIM_N - 1] = code_c[347];
    idx = 346;
    for(i = 0; i < 5; i++)
    {
        tmp[72] = tmp[72] << 8;
        tmp[72] += code_c[idx--];
    }

    idx = 288;
    for(i = 0; i < 72; i += 4)
    {
        tmp[i] = ((code_c[idx + 2] & 15) << 8) + code_c[idx];
        tmp[i] = tmp[i] << 26;
        tmp[i + 2] = ((code_c[idx + 2] >> 4) << 8) + code_c[idx + 1];
        tmp[i + 2] = tmp[i + 2] << 26;
        idx += 3;
    }

    res = code_c;
    j = 0;
    for(i = 0; i < 72; i += 2)
    {
        tmp[i] += (res[j] & 67108863);
        tmp[i + 1] = (res[j++] >> 26);
    }

    idx = 0;
    for(i = 0; i < DIM_N - 1; i += 7)
    {
        for(j = 0; j < 7; j++)
        {
            tmp_c1[i + j] = (uint16_t)(tmp[idx] % 43);
            tmp[idx] = tmp[idx] / 43;
        }
        idx++;
    }

    tmp_32[30] = code_c[449] >> 2;
    t = ((code_c[449] & 3) << 8) + code_c[448];

    res_32 = code_c + 348;

    j = 0;
    for(i = 0; i < 30; i += 6)
    {
        tmp_32[i] = res_32[j] & 134217727;
        tmp_32[i + 1] = res_32[j + 1] & 134217727;
        tmp_32[i + 2] = res_32[j + 2] & 134217727;
        tmp_32[i + 3] = res_32[j + 3] & 134217727;
        tmp_32[i + 4] = res_32[j + 4] & 134217727;
        tmp_32[i + 5] = ((res_32[j] >> 27) | ((res_32[j + 1] >> 27) << 5) | ((res_32[j + 2] >> 27) << 10) | ((res_32[j + 3] >> 27) << 15) | ((res_32[j + 4] >> 27) << 20));
        j += 5;
    }

    tmp_32[5] += ((t >> 8) << 25);
    tmp_32[11] += (((t >> 6) & 3) << 25);
    tmp_32[17] += (((t >> 4) & 3) << 25);
    tmp_32[23] += (((t >> 2) & 3) << 25);
    tmp_32[29] += ((t & 3) << 25);

    for(i = 0; i < 2; i++)
    {
        tmp_c2[DIM_N - 2 + i] = (uint16_t)(tmp_32[30] % 3);
        tmp_32[30] = tmp_32[30] / 3;
    }

    idx = 0;
    for(i = 0; i < DIM_N - 2; i += 17)
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
        c[i] = (tmp_c1[i] * 3 * 29 + tmp_c2[i] * 43) % 129;
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