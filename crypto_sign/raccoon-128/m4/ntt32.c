//  ntt32.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === 32-bit Number Theoretic Transform

#include <stddef.h>
#include <stdbool.h>

#include "polyr.h"

//  2x32 CRT: Split into two-prime representation (in-place).
extern void polyr2_split_asm(int64_t* v);
void polyr2_split(int64_t *v)
{
    polyr2_split_asm(v);
}

//  2x32 CRT: Join two-prime into 64-bit integer representation (in-place).
//  Use scale factors (s1, s2). Normalizes to 0 <= x < q.

extern void polyr2_join_asm(int64_t* v, int32_t s1, int32_t s2);
void polyr2_join(int64_t *v, int32_t s1, int32_t s2)
{
    polyr2_join_asm(v,s1,s2);
}

//  2x32 CRT: Add polynomials:  r = a + b.
extern void polyr2_add_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr2_add(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr2_add_asm(r,a,b);
}

//  2x32 CRT: Subtract polynomials:  r = a - b.
extern void polyr2_sub_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr2_sub(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr2_sub_asm(r,a,b);
}

//  2x32 CRT: Scalar multiplication:    r = a * c,  Montgomery reduction.
extern void polyr_ntt_smul_asm(int64_t *r, const int64_t *a, int32_t c1, int32_t c2);
void polyr_ntt_smul(int64_t *r, const int64_t *a, int32_t c1, int32_t c2)
{
    polyr_ntt_smul_asm(r,a,c1,c2);
}

//  2x32 CRT: Coefficient multiply:  r = a * b,  Montgomery reduction.
extern void polyr_ntt_cmul_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_ntt_cmul(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_ntt_cmul_asm(r, a, b);
}

//  2x32 CRT: Multiply and add:  r = a * b + c, Montgomery reduction.
extern void polyr_ntt_mula_asm(int64_t *r, const int64_t *a, const int64_t *b, const int64_t *c);
void polyr_ntt_mula(int64_t *r, const int64_t *a, const int64_t *b,
                    const int64_t *c)
{
    polyr_ntt_mula_asm(r, a, b, c);
}

//  2x32 CRT: Forward NTT (x^n+1). Input is 64-bit, output is 2x32 CRT.
extern void raccoon_ntt(int64_t p[512]);
void polyr_fntt(int64_t *v)
{
    raccoon_ntt(v);
}

//  2x32 CRT: Inverse NTT (x^n+1).
extern void raccoon_invntt(int64_t *v);
void polyr_intt(int64_t *v)
{
    raccoon_invntt(v);
}
