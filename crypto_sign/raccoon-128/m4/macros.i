// Based on the code from Dilithium M4

#ifndef MACROS_I
#define MACROS_I
// 3
.macro montgomery_mul_32 a, b, Qprime, Q, tmp, tmp2
    smull \tmp, \a, \a, \b
    mul \tmp2, \tmp, \Qprime
    smlal \tmp, \a, \tmp2, \Q
.endm

// conditional addition if a is negative number
.macro mont32_cadd_asm a, m, tmp
    and \tmp, \m, \a, asr #31
    add \a, \tmp
.endm

.macro mont64_sub_asm aLo, aHi, bLo, bHi
    subs \aLo, \aLo, \bLo
    sbc \aHi, \aHi, \bHi
.endm

.macro mont64_add_asm aLo, aHi, bLo, bHi
    adds \aLo, \aLo, \bLo
    adc \aHi, \aHi, \bHi
.endm

.macro mont64_neg_asm aLo, aHi
    negs \aLo, \aLo
    sbc \aHi, \aHi, \aHi, lsl #1
.endm

.macro mont64_csub_asm aLo, aHi, mLo, mHi, tmp, tmp2
    subs \aLo, \aLo, \mLo
    sbc \aHi, \aHi, \mHi
    and \tmp, \mLo, \aHi, asr #31
    and \tmp2, \mHi, \aHi, asr #31
    adds \aLo, \aLo, \tmp
    adc \aHi, \aHi, \tmp2
.endm

.macro mont64_cadd_asm aLo, aHi, mLo, mHi, tmp, tmp2
    and \tmp, \mLo, \aHi, asr #31
    and \tmp2, \mHi, \aHi, asr #31
    adds \aLo, \aLo, \tmp
    adc \aHi, \aHi, \tmp2
.endm

.macro reduce64_asm aLo, aHi, qLo, qHi, tmp, tmp2, tmp3
    add \tmp, \aHi, #65536
    asr \tmp, \tmp, #17
    smull \tmp2, \tmp3, \tmp, \qLo
    mla \tmp3, \tmp, \qHi, \tmp3
    subs \aLo, \tmp2
    sbc \aHi, \tmp3
.endm

// aLo and aHi are modified, result in aHi. need more instruction to achieve in-place
.macro montgomery_redc_v1 aLo, aHi, Qprime1, Q1, Qprime2, Q2, tmp, tmp2, tmp3
    mul \tmp, \aLo, \Qprime2
    mov \tmp2, \aLo
    mov \tmp3, \aHi 
    smlal \aLo, \aHi, \tmp, \Q2// res in aHi
    mul \tmp, \tmp2, \Qprime1
    smlal \tmp2, \tmp3, \tmp, \Q1 //we need res in aLo
    mov \aLo, \tmp3
.endm

// verified & in-place; here we use q^-1 mod 2^32 for in-place implementation
.macro montgomery_redc_v2 aLo, aHi, Qprime1, Q1, Qprime2, Q2, tmp, tmp2
    mul \tmp, \aLo, \Qprime1
    mul \tmp2, \aLo, \Qprime2 // aLo can be reused now.
    smmul \tmp, \tmp, \Q1
    smmul \tmp2, \tmp2, \Q2
    sub \aLo, \aHi, \tmp
    sub \aHi, \aHi, \tmp2
.endm

// here we use q^-1 mod 2^32 for in-place implementation; the results are -aLo and -aHi
.macro montgomery_redc_v3 aLo, aHi, Qprime1, Q1, Qprime2, Q2, tmp, tmp2
    mul \tmp, \aLo, \Qprime1
    mul \tmp2, \aLo, \Qprime2 // aLo can be reused now.
    neg.w \aHi, \aHi
    smmla \aLo, \tmp, \Q1, \aHi
    smmla \aHi, \tmp2, \Q2, \aHi
.endm

// use -Q1, -Q2 and smmla; use q^-1, q; not working; minor difference
.macro montgomery_redc_v4 aLo, aHi, Qprime1, Q1, Qprime2, Q2, tmp, tmp2
    mul \tmp, \aLo, \Qprime1
    mul \tmp2, \aLo, \Qprime2 // aLo can be reused now.
    smmls \aLo, \tmp, \Q1, \aHi 
    smmls \aHi, \tmp2, \Q2, \aHi
.endm

// 2
.macro addSub1 c0, c1
    add.w \c0, \c1
    sub.w \c1, \c0, \c1, lsl #1
.endm

// 3
.macro addSub2 c0, c1, c2, c3
    add \c0, \c1
    add \c2, \c3
    sub.w \c1, \c0, \c1, lsl #1
    sub.w \c3, \c2, \c3, lsl #1
.endm

// 6
.macro addSub4 c0, c1, c2, c3, c4, c5, c6, c7
    add \c0, \c1
    add \c2, \c3
    add \c4, \c5
    add \c6, \c7
    sub.w \c1, \c0, \c1, lsl #1
    sub.w \c3, \c2, \c3, lsl #1
    sub.w \c5, \c4, \c5, lsl #1
    sub.w \c7, \c6, \c7, lsl #1
.endm

.macro _3_layer_CT_32 c0, c1, c2, c3, c4, c5, c6, c7, xi0, xi1, xi2, xi3, xi4, xi5, xi6, twiddle, Qprime, Q, tmp, tmp2
    vmov.w \twiddle, \xi0
    montgomery_mul_32 \c4, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c5, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c4, \c1, \c5, \c2, \c6, \c3, \c7

    vmov.w \twiddle, \xi1
    montgomery_mul_32 \c2, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi2
    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c2, \c1, \c3, \c4, \c6, \c5, \c7

    vmov.w \twiddle, \xi3
    montgomery_mul_32 \c1, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi4
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi5
    montgomery_mul_32 \c5, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi6
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c1, \c2, \c3, \c4, \c5, \c6, \c7
.endm

.macro _3_layer_inv_CT_32 c0, c1, c2, c3, c4, c5, c6, c7, xi0, xi1, xi2, xi3, xi4, xi5, xi6, twiddle, Qprime, Q, tmp, tmp2
    vmov.w \twiddle, \xi0
    montgomery_mul_32 \c1, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c5, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c1, \c2, \c3, \c4, \c5, \c6, \c7

    vmov.w \twiddle, \xi1
    montgomery_mul_32 \c2, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi2
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c2, \c1, \c3, \c4, \c6, \c5, \c7

    vmov.w \twiddle, \xi3
    montgomery_mul_32 \c4, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi4
    montgomery_mul_32 \c5, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi5
    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov.w \twiddle, \xi6
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub4 \c0, \c4, \c1, \c5, \c2, \c6, \c3, \c7
.endm


.macro _3_layer_inv_twist_CT_32 c0, c1, c2, c3, c4, c5, c6, c7, xi0, xi1, xi2, xi3, xi4, xi5, xi6, xi7, twiddle, Qprime, Q, tmp, tmp2
    vmov \twiddle, \xi0 
    montgomery_mul_32 \c0, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi1
    montgomery_mul_32 \c1, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi2
    montgomery_mul_32 \c2, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi3
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi4
    montgomery_mul_32 \c4, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi5
    montgomery_mul_32 \c5, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi6
    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2
    vmov \twiddle, \xi7
    montgomery_mul_32 \c7, \twiddle, \Qprime, \Q, \tmp, \tmp2
.endm

/************************************************************
* Name:         _3_layer_inv_butterfly_light_fast_first
*
* Description:  upper half of 3-layer inverse butterfly
*               defined over X^8 - 1
*
* Input:        (c4, c1, c6, c3) = coefficients on the upper half;
*               (xi0, xi1, xi2, xi3, xi4, xi5, xi6) =
*               (  1,  1,  w_4,   1, w_8, w_4, w_8^3) in
*               Montgomery domain
*
* Symbols:      R = 2^32
*
* Constants:    Qprime = -MOD^{-1} mod^{+-} R, Q = MOD
*
* Output:
*               c4 =  c4 + c1        + (c6 + c3)
*               c5 = (c4 - c1) w_4   + (c6 + c3) w_8^3
*               c6 =  c4 + c1        - (c6 + c3)
*               c7 = (c4 - c1) w_8^3 + (c6 + c3) w_4
************************************************************/
// 15
.macro _3_layer_inv_butterfly_light_fast_first c0, c1, c2, c3, c4, c5, c6, c7, xi0, xi1, xi2, xi3, xi4, xi5, xi6, twiddle, Qprime, Q, tmp, tmp2
    addSub2 \c4, \c1, \c6, \c3
    addSub1 \c4, \c6

    vmov.w \tmp, \xi4
    vmov.w \tmp2, \xi6

    smull.w \c0, \c5, \c1, \tmp
    smlal.w \c0, \c5, \c3, \tmp2
    mul.w \twiddle, \c0, \Qprime
    smlal.w \c0, \c5, \twiddle, \Q

    smull.w \c2, \c7, \c1, \tmp2
    smlal.w \c2, \c7, \c3, \tmp
    mul.w \twiddle, \c2, \Qprime
    smlal.w \c2, \c7, \twiddle, \Q
.endm

/************************************************************
* Name:         _3_layer_inv_butterfly_light_fast_second
*
* Description:  lower half of 3-layer inverse butterfly
*               defined over X^8 - 1, and the 2nd
*               layer of butterflies
*
* Input:
*               (c4, c5, c6, c7) = results of the upper half;
*               (c0, c1, c2, c3) = coefficients on the lower half;
*               (xi0, xi1, xi2, xi3, xi4, xi5, xi6) =
*               (  1,  1,  w_4,   1, w_8, w_4, w_8^3) in
*               Montgomery domain
*
* Symbols:      R = 2^32
*
* Constants:    Qprime = -MOD^{-1} mod^{+-} R, Q = MOD
*
* Output:       (normal order)
*               c0 =   c0 + c1     + (c2 + c3)         + (  c4 + c5     + (c6 + c7)       )
*               c1 =  (c0 - c1) w3 + (c2 - c3)  w4     + ( (c4 - c5) w5 + (c6 - c7) w6    )
*               c2 = ( c0 + c1     - (c2 + c3)) w1     + (( c4 + c5     - (c6 + c7)   ) w2)
*               c3 = ((c0 - c1) w3 - (c2 - c3)  w4) w1 + (((c4 - c5) w5 - (c6 - c7) w6) w2)
*               c4 =   c0 + c1     - (c2 + c3)         - (  c4 + c5     + (c6 + c7)       ) w0
*               c5 =  (c0 - c1) w3 + (c2 - c3)  w4     - ( (c4 - c5) w5 + (c6 - c7) w6    ) w0
*               c6 = ( c0 + c1     - (c2 + c3)) w1     - (( c4 + c5     - (c6 + c7)   ) w2) w0
*               c7 = ((c0 - c1) w3 - (c2 - c3)  w4) w1 - (((c4 - c5) w5 - (c6 - c7) w6) w2) w0
************************************************************/
// 19
.macro _3_layer_inv_butterfly_light_fast_second c0, c1, c2, c3, c4, c5, c6, c7, xi0, xi1, xi2, xi3, xi4, xi5, xi6, twiddle, Qprime, Q, tmp, tmp2
    addSub2 \c0, \c1, \c2, \c3

    vmov.w \twiddle, \xi2
    montgomery_mul_32 \c3, \twiddle, \Qprime, \Q, \tmp, \tmp2
    addSub2 \c0, \c2, \c1, \c3

    montgomery_mul_32 \c6, \twiddle, \Qprime, \Q, \tmp, \tmp2

    addSub4 \c0, \c4, \c1, \c5, \c2, \c6, \c3, \c7
.endm

#endif /* MACROS_I */
