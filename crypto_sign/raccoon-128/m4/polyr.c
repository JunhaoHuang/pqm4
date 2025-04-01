//  polyr.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Polynomial arithmetic related to the ring Zq[x]/(x^n+1).

#include <stddef.h>
#include <stdbool.h>

#include "polyr.h"
#include "mont64.h"
#include "mont32.h"

//  === Polynomial API

//  Zeroize a polynomial:   r = 0.

void polyr_zero(int64_t *r)
{
    size_t i;

    for (i = 0; i < RACC_N; i++) {
        r[i] = 0;
    }
}

//  Copy a polynomial:  r = a.

void polyr_copy(int64_t *r, const int64_t *a)
{
    size_t i;

    for (i = 0; i < RACC_N; i++) {
        r[i] = a[i];
    }
}

//  Add polynomials:  r = a + b.
extern void polyr_add_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_add(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_add_asm(r,a,b);
}

//  Subtract polynomials:  r = a - b.
extern void polyr_sub_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_sub(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_sub_asm(r,a,b);
}

//  Add polynomials mod q:  r = a + b  (mod q).
extern void polyr_addq_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_addq(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_addq_asm(r,a,b);
}


//  Subtract polynomials mod q:  r = a - b  (mod q).
extern void polyr_subq_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_subq(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_subq_asm(r,a,b);
}


//  Add polynomials:  r = a + b, conditionally subtract m on overflow
extern void polyr_addm_asm(int64_t *r, const int64_t *a, const int64_t *b, int64_t* m);
void polyr_addm(int64_t *r, const int64_t *a, const int64_t *b, int64_t m)
{
    polyr_addm_asm(r,a,b,&m);
}

//  Subtract polynomials:  r = a - b, conditionally add m on underflow.
extern void polyr_subm_asm(int64_t *r, const int64_t *a, const int64_t *b, int64_t* m);
void polyr_subm(int64_t *r, const int64_t *a, const int64_t *b, int64_t m)
{
    polyr_subm_asm(r,a,b,&m);
}

//  Negate a polynomial mod m:  r = -a, add m on underflow.
extern void polyr_negm_asm(int64_t *r, int64_t *a, int64_t m);
void polyr_negm(int64_t *r, int64_t *a, int64_t m)
{
    polyr_negm_asm(r,a,m);
}
extern void polyr_neg_asm(int64_t *r, const int64_t *a);
void polyr_neg(int64_t *r, int64_t *a)
{
    polyr_neg_asm(r,a);
}

//  Left shift:  r = a << sh, conditionally subtract m on overflow.
extern void polyr_shlm42_asm(int64_t *r, const int64_t *a, int64_t m);
void polyr_shlm(int64_t *r, const int64_t *a, size_t sh, int64_t m)
{
    XASSUME(sh==42);
    (void)sh;
    polyr_shlm42_asm(r,a,m);
}

//  Right shift:  r = a >> sh, conditionally subtract m on overflow.

void polyr_shrm(int64_t *r, const int64_t *a, size_t sh, int64_t m)
{
    size_t i;

    for (i = 0; i < RACC_N; i++) {
        r[i] = mont64_csub(a[i] >> sh, m);
    }
}

//  Rounding:  r = (a + h) >> sh, conditionally subtract m on overflow.

void polyr_round(int64_t *r, const int64_t *a, size_t sh, int64_t h, int64_t m)
{
    size_t i;

    for (i = 0; i < RACC_N; i++) {
        r[i] = mont64_csub((a[i] + h) >> sh, m);
    }
}

//  Move from range 0 <= x < m to centered range -m/2 <= x <  m/2.
extern void polyr_center_asm(int64_t *r, const int64_t *a, int64_t m);
void polyr_center(int64_t *r, const int64_t *a, int64_t m)
{
    polyr_center_asm(r,a,m);
}

//  Move from range -m <= x < m to non-negative range 0 <= x < m.
extern void polyr_nonneg_asm(int64_t *r, const int64_t *a, int64_t m);
void polyr_nonneg(int64_t *r, const int64_t *a, int64_t m)
{
    polyr_nonneg_asm(r,a,m);
}

extern void polyr_reduce_asm(int64_t *r, const int64_t *a); 
// With conditional addition to reduce results in [0,q)
void polyr_reduce(int64_t *r, const int64_t *a)
{
    polyr_reduce_asm(r,a);
}
