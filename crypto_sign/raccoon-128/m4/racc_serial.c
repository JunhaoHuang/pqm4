//  racc_serial.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === Raccoon signature scheme -- Serialize/deserialize.

#include <string.h>
#include "racc_core.h"
#include "racc_serial.h"
#include "plat_local.h"
#include "polyr.h"
#include "xof_sample.h"
#include "randombytes.h"
#include "mont32.h"
#include "mont64.h"
#include "fips202.h"

//  Encode vector v[RACC_N] as packed "bits" sized elements to  *b".
//  Return the number of bytes written -- at most ceil(RACC_N * bits/8).

static inline size_t inline_encode_bits(uint8_t *b, const int64_t v[RACC_N],
                                        size_t bits)
{
    size_t i, j, l;
    int64_t x, m;

    i = 0;  //  source word v[i]
    j = 0;  //  destination byte b[j]
    l = 0;  //  number of bits in x
    x = 0;  //  bit buffer

    m = (1llu << bits) - 1llu;

    while (i < RACC_N) {
        while (l < 8 && i < RACC_N) {
            x |= (v[i++] & m) << l;
            l += bits;
        }
        while (l >= 8) {
            b[j++] = (uint8_t)(x & 0xFF);
            x >>= 8;
            l -= 8;
        }
    }
    if (l > 0) {
        b[j++] = (uint8_t)(x & 0xFF);
    }

    return j;  //   return number of bytes written
}

//  Decode bytes from "*b" as RACC_N vector elements of "bits" each.
//  The decoding is unsigned if "is_signed"=false, two's complement
//  signed representation assumed if "is_signed"=true. Return the
//  number of bytes read -- upper bounded by ceil(RACC_N * bits/8).

static inline size_t inline_decode_bits(int64_t v[RACC_N], const uint8_t *b,
                                        size_t bits, bool is_signed)
{
    size_t i, j, l;
    int64_t x, m, s;

    i = 0;  //  source byte b[i]
    j = 0;  //  destination word v[j]
    l = 0;  //  number of bits in x
    x = 0;  //  bit buffer

    if (is_signed) {
        s = 1llu << (bits - 1);  // extract sign bit
        m = s - 1;
    } else {
        s = 0;  //  sign bit ignored
        m = (1llu << bits) - 1;
    }

    while (j < RACC_N) {

        while (l < bits) {
            x |= ((uint64_t)b[i++]) << l;
            l += 8;
        }
        while (l >= bits && j < RACC_N) {
            v[j++] = (x & m) - (x & s);
            x >>= bits;
            l -= bits;
        }
    }

    return i;  //   return number of bytes read
}

//  === Interface

//  Encode the public key "pk" to bytes "b". Return length in bytes.

size_t racc_encode_pk(uint8_t *b, const racc_pk_t *pk)
{
    size_t i, l;

    l = 0; //  l holds the length

    //  encode A seed
    memcpy(b + l, pk->a_seed, RACC_AS_SZ);
    l += RACC_AS_SZ;

    //  encode t vector
    for (i = 0; i < RACC_K; i++)
    {
        //  domain is q_t; has log2(q) - log(p_t) bits
        l += inline_encode_bits(b + l, pk->t[i], RACC_Q_BITS - RACC_NUT);
    }

    return l;
}

//  Decode a public key from "b" to "pk". Return length in bytes.

size_t racc_decode_pk(racc_pk_t *pk, const uint8_t *b)
{
    size_t i, l;

    l = 0;

    //  decode A seed
    memcpy(pk->a_seed, b + l, RACC_AS_SZ);
    l += RACC_AS_SZ;

    //  decode t vector
    for (i = 0; i < RACC_K; i++)
    {
        //  domain is q_t; has log2(q) - log(p_t) bits, unsigned
        l += inline_decode_bits(pk->t[i], b + l, RACC_Q_BITS - RACC_NUT, false);
    }

    //  also set the tr field
    shake256(pk->tr, RACC_TR_SZ, b, l);

    return l;
}

#if MEM_OPT > 0
size_t racc_encode_pk_k(uint8_t *b, const int64_t *t, size_t l)
{
    l += inline_encode_bits(b + l, t, RACC_Q_BITS - RACC_NUT);
    return l;
}
size_t racc_decode_pk_k(int64_t pk[RACC_N], const uint8_t *b, size_t l)
{
    //  decode t vector
    //  domain is q_t; has log2(q) - log(p_t) bits, unsigned
    l += inline_decode_bits(pk, b + l, RACC_Q_BITS - RACC_NUT, false);
    return l;
}

//  Encode D-share of the i-th polynomial "s" of RACC_ELL-degree "sk" to bytes "b". Return length in bytes.
size_t racc_encode_sk_l(uint8_t *b, const int64_t s[RACC_D][RACC_N], size_t i, size_t lk, size_t ls)
{
    size_t j;
    uint8_t buf[RACC_MK_SZ + 8];
    int64_t r[RACC_N], s0[RACC_N];

    //  make a copy of share 0
    polyr2_neg(s0, s[0]);
    polyr2_join(s0, MONT_D2Q1, MONT_D2Q2);

    memset(buf, 0x00, 8); //   domain header template
    buf[0] = 'K';
    buf[1] = i; // update domain header

    //  shares 1, 2, ..., d-1
    for (j = 1; j < RACC_D; j++)
    {
        memcpy(buf + 8, b + lk, RACC_MK_SZ); // store in secret key
        lk += RACC_MK_SZ;

        //  XOF( 'K' || index i || share j || key_j )

        buf[2] = j;

        xof_sample_q(r, buf, RACC_MK_SZ + 8);
        polyr_subq(s0, s0, r); //    s0 <- s0 - r
        polyr2_neg(r, s[j]);
        polyr2_join(r, MONT_D2Q1, MONT_D2Q2);
        polyr_addq(s0, s0, r); //    s0 <- s0 + s_j
    }
    // polyr_reduce(s0,s0);
    //  encode the zeroth share (in full)
    ls += inline_encode_bits(b + ls, s0, RACC_Q_BITS);

    return ls;
}

//  Decode D-share of the i-th polynomial "s" of bytes "b" to "sk". Return length in bytes.
size_t racc_decode_sk_l(int64_t sk[RACC_D][RACC_N], const uint8_t *b, size_t i, size_t lk, size_t ls)
{
    size_t j;
    uint8_t buf[RACC_MK_SZ + 8];

    memset(buf, 0x00, 8); //   domain header template
    buf[0] = 'K';

    //  expand shares 1, 2, ..., d-1 from keys
    for (j = 1; j < RACC_D; j++)
    {
        //  copy key
        memcpy(buf + 8, b + lk, RACC_MK_SZ);
        lk += RACC_MK_SZ;

        //  XOF( 'K' || i || share j || key_j )
        buf[1] = i; // update domain header
        buf[2] = j;
        xof_sample_q(sk[j], buf, RACC_MK_SZ + 8);
    }

    //  decode the zeroth share (in full)
    ls += inline_decode_bits(sk[0], b + ls, RACC_Q_BITS, false);

    //  convert S
    for (j = 0; j < RACC_D; j++)
    {
        polyr2_split_neg(sk[j]);
    }

    return ls;
}
#else
//  Encode secret key "sk" to bytes "b". Return length in bytes.
size_t racc_encode_sk(uint8_t *b, const racc_sk_t *sk)
{
    size_t i, j, l;
    uint8_t buf[RACC_MK_SZ + 8];
    int64_t r[RACC_N], s0[RACC_ELL][RACC_N];
    int64_t t[RACC_N];

    //  encode public key
    l = racc_encode_pk(b, &sk->pk);

    //  make a copy of share 0
    for (i = 0; i < RACC_ELL; i++)
    {
        polyr2_neg(s0[i], sk->s[i][0]);
        polyr2_join(s0[i], MONT_D2Q1, MONT_D2Q2);
    }

    memset(buf, 0x00, 8); //   domain header template
    buf[0] = 'K';

    //  shares 1, 2, ..., d-1
    for (j = 1; j < RACC_D; j++)
    {

        randombytes(b + l, RACC_MK_SZ);     //   key_j
        memcpy(buf + 8, b + l, RACC_MK_SZ); // store in secret key
        l += RACC_MK_SZ;

        //  XOF( 'K' || index i || share j || key_j )
        for (i = 0; i < RACC_ELL; i++)
        {
            buf[1] = i; // update domain header
            buf[2] = j;

            xof_sample_q(r, buf, RACC_MK_SZ + 8);
            polyr_subq(s0[i], s0[i], r); //    s0 <- s0 - r
            polyr2_neg(t, sk->s[i][j]);
            polyr2_join(t, MONT_D2Q1, MONT_D2Q2);
            polyr_addq(s0[i], s0[i], t); //    s0 <- s0 + s_j
        }
    }
    // polyr_reduce(s0[i], s0[i]);
    //  encode the zeroth share (in full)
    for (i = 0; i < RACC_ELL; i++)
    {
        l += inline_encode_bits(b + l, s0[i], RACC_Q_BITS);
    }

    return l;
}

//  Decode secret key "sk" to bytes "b". Return length in bytes.
size_t racc_decode_sk(racc_sk_t *sk, const uint8_t *b)
{
    size_t i, j, l;
    uint8_t buf[RACC_MK_SZ + 8];

    //  decode public key
    l = racc_decode_pk(&sk->pk, b);

    memset(buf, 0x00, 8); //   domain header template
    buf[0] = 'K';

    //  expand shares 1, 2, ..., d-1 from keys
    for (j = 1; j < RACC_D; j++)
    {

        //  copy key
        memcpy(buf + 8, b + l, RACC_MK_SZ);
        l += RACC_MK_SZ;

        //  XOF( 'K' || i || share j || key_j )
        for (i = 0; i < RACC_ELL; i++)
        {
            buf[1] = i; // update domain header
            buf[2] = j;
            xof_sample_q(sk->s[i][j], buf, RACC_MK_SZ + 8);
        }
    }

    //  decode the zeroth share (in full)
    for (i = 0; i < RACC_ELL; i++)
    {
        l += inline_decode_bits(sk->s[i][0], b + l, RACC_Q_BITS, false);
    }

    //  convert S
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_D; j++)
        {
            polyr2_split_neg(sk->s[i][j]);
        }
    }

    return l;
}
#endif

//  macro for encoding n bits from y
//  (note -- returns from function on overflow)
#define ENC_SIG_PUT_BITS(y,n) { \
    while (n > 0) {             \
        n--;                    \
        z |= (y & 1) << k;      \
        y >>= 1;                \
        k++;                    \
        if (k == 8) {           \
            if (l >= b_sz)      \
                return 0;       \
            b[l++] = z;         \
            k = 0;              \
            z = 0;              \
        }                       \
    }                           \
}

#if MEM_OPT == 2 // on-the-fly decode/encode; encode/decode h before z.
size_t racc_encode_sig_h(uint8_t *b, size_t i, size_t b_sz, size_t l_h, uint8_t *pre_z, size_t *pre_k, const int64_t sig_h[RACC_N])
{
    size_t j, k, l, n;
    int64_t x, y, s;
    uint8_t z;

    l = l_h;
    k = *pre_k;      //  bit position 0..7
    z = *pre_z;      //  byte fraction

    //  encode i-th hint
    for (j = 0; j < RACC_N; j++)
    {
        //  normalize
        x = sig_h[j];
        while (x < -RACC_Q / 2)
            x += RACC_Q;
        while (x > RACC_Q / 2)
            x -= RACC_Q;

        if (x == 0)
        {
            //  zero is encoded just as one zero bit
            y = 0;
            n = 1;
        }
        else
        {
            //  set sign
            if (x < 0)
            {
                x = -x;
                s = 1;
            }
            else
            {
                s = 0;
            }
            //  abs(x) reps of 1, followed by 0 stop bit and sign
            y = ((1LL << x) - 1) | (s << (x + 1));
            n = x + 2;
        }

        //  encode n bits from y
        ENC_SIG_PUT_BITS(y, n);
    }
    //  fractional byte at final iterations
    if (i == (RACC_K - 1))
    {
        if (k > 0)
        {
            b[l++] = z;
        }
    }

    *pre_k=k;
    *pre_z=z;

    return l;
}

size_t racc_encode_sig_z(uint8_t *b, size_t b_sz, size_t l_z, uint8_t *pre_z, size_t *pre_k, const int64_t sig_z[RACC_N])
{
    size_t j, k, l, n;
    int64_t x, y, s;
    uint8_t z;

    l = l_z; //  byte position (length)
    k = *pre_k;          //  bit position 0..7
    z = *pre_z;       //  byte fraction

    //  encode z
    for (j = 0; j < RACC_N; j++)
    {
        x = sig_z[j];

        //  normalize
        while (x < -RACC_Q / 2)
            x += RACC_Q;
        while (x > RACC_Q / 2)
            x -= RACC_Q;

        //  set sign
        if (x < 0)
        {
            x = -x;
            s = 1;
        }
        else
        {
            s = 0;
        }

        //  low bits
        y = x & ((1LL << RACC_ZLBITS) - 1);
        x >>= RACC_ZLBITS;

        //  high bits (run of 1's)
        y |= ((1LL << x) - 1) << RACC_ZLBITS;

        if (y == 0)
        {
            //  stop bit, no sign
            n = RACC_ZLBITS + 1;
        }
        else
        {
            //  stop bit (0) and sign
            y |= s << (RACC_ZLBITS + x + 1);
            n = RACC_ZLBITS + x + 2;
        }

        //  encode n bits from y
        ENC_SIG_PUT_BITS(y, n);
    }

    *pre_k = k;
    *pre_z = z;

    return l;
}
#elif MEM_OPT == 1 // follow the origin signature encode/decode
size_t racc_encode_sig_zh(uint8_t *b, size_t b_sz, const int64_t sig_h[RACC_K][RACC_N], const int64_t sig_z[RACC_ELL][RACC_N])
{
    size_t i, j, k, l, n;
    int64_t x, y, s;
    uint8_t z;

    l = RACC_CH_SZ; //  byte position (length)
    k = 0;          //  bit position 0..7
    z = 0x00;       //  byte fraction

    //  encode hint
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {

            //  normalize
            x = sig_h[i][j];
            while (x < -RACC_Q / 2)
                x += RACC_Q;
            while (x > RACC_Q / 2)
                x -= RACC_Q;

            if (x == 0)
            {
                //  zero is encoded just as one zero bit
                y = 0;
                n = 1;
            }
            else
            {
                //  set sign
                if (x < 0)
                {
                    x = -x;
                    s = 1;
                }
                else
                {
                    s = 0;
                }
                //  abs(x) reps of 1, followed by 0 stop bit and sign
                y = ((1LL << x) - 1) | (s << (x + 1));
                n = x + 2;
            }

            //  encode n bits from y
            ENC_SIG_PUT_BITS(y, n);
        }
    }

    //  encode z
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = sig_z[i][j];

            //  normalize
            while (x < -RACC_Q / 2)
                x += RACC_Q;
            while (x > RACC_Q / 2)
                x -= RACC_Q;

            //  set sign
            if (x < 0)
            {
                x = -x;
                s = 1;
            }
            else
            {
                s = 0;
            }

            //  low bits
            y = x & ((1LL << RACC_ZLBITS) - 1);
            x >>= RACC_ZLBITS;

            //  high bits (run of 1's)
            y |= ((1LL << x) - 1) << RACC_ZLBITS;

            if (y == 0)
            {
                //  stop bit, no sign
                n = RACC_ZLBITS + 1;
            }
            else
            {
                //  stop bit (0) and sign
                y |= s << (RACC_ZLBITS + x + 1);
                n = RACC_ZLBITS + x + 2;
            }

            //  encode n bits from y
            ENC_SIG_PUT_BITS(y, n);
        }
    }

    //  fractional byte
    if (k > 0)
    {
        if (l >= b_sz)
            return 0;
        b[l++] = z;
    }

    return l;
}

#else
//  Encode signature "sig" to "*b" of max "b_sz" bytes. Return length in
//  bytes or zero in case of overflow.

size_t racc_encode_sig(uint8_t *b, size_t b_sz, const racc_sig_t *sig)
{
    size_t i, j, k, l, n;
    int64_t x, y, s;
    uint8_t z;

    //  encode challenge hash
    memcpy(b, sig->ch, RACC_CH_SZ);

    l = RACC_CH_SZ; //  byte position (length)
    k = 0;          //  bit position 0..7
    z = 0x00;       //  byte fraction

    //  encode hint
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {

            //  normalize
            x = sig->h[i][j];
            while (x < -RACC_Q / 2)
                x += RACC_Q;
            while (x > RACC_Q / 2)
                x -= RACC_Q;

            if (x == 0)
            {
                //  zero is encoded just as one zero bit
                y = 0;
                n = 1;
            }
            else
            {
                //  set sign
                if (x < 0)
                {
                    x = -x;
                    s = 1;
                }
                else
                {
                    s = 0;
                }
                //  abs(x) reps of 1, followed by 0 stop bit and sign
                y = ((1LL << x) - 1) | (s << (x + 1));
                n = x + 2;
            }

            //  encode n bits from y
            ENC_SIG_PUT_BITS(y, n);
        }
    }

    //  encode z
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = sig->z[i][j];

            //  normalize
            while (x < -RACC_Q / 2)
                x += RACC_Q;
            while (x > RACC_Q / 2)
                x -= RACC_Q;

            //  set sign
            if (x < 0)
            {
                x = -x;
                s = 1;
            }
            else
            {
                s = 0;
            }

            //  low bits
            y = x & ((1LL << RACC_ZLBITS) - 1);
            x >>= RACC_ZLBITS;

            //  high bits (run of 1's)
            y |= ((1LL << x) - 1) << RACC_ZLBITS;

            if (y == 0)
            {
                //  stop bit, no sign
                n = RACC_ZLBITS + 1;
            }
            else
            {
                //  stop bit (0) and sign
                y |= s << (RACC_ZLBITS + x + 1);
                n = RACC_ZLBITS + x + 2;
            }

            //  encode n bits from y
            ENC_SIG_PUT_BITS(y, n);
        }
    }

    //  fractional byte
    if (k > 0)
    {
        if (l >= b_sz)
            return 0;
        b[l++] = z;
    }

    return l;
}
#endif

#undef ENC_SIG_PUT_BITS

//  macro that gets a single bit
#define DEC_SIG_GET_BIT(bit) {  \
    bit = (z >> k) & 1;         \
    k++;                        \
    if (k == 8) {               \
        if (l >= b_sz)          \
            return 0;           \
        z = b[l++];             \
        k = 0;                  \
    }                           \
}

//  decode bytes "b" into signature "sig". Return length in bytes.
// different decode sequence for memory optimization
#if MEM_OPT == 2
// separately decode z to reduce memory for vz.
size_t racc_decode_sig_z(int64_t sig_z[RACC_N], size_t b_sz, size_t l_z, uint8_t *pre_z, size_t *pre_k, const uint8_t *b)
{
    size_t j, k, l, n;
    uint8_t bit, z;
    int64_t x;

    l = l_z;

    z = *pre_z;
    k = *pre_k;
    //  decode z
    for (j = 0; j < RACC_N; j++)
    {
        x = 0; //  get low bits
        for (n = 0; n < RACC_ZLBITS; n++)
        {
            DEC_SIG_GET_BIT(bit)
            x |= ((int64_t)bit) << n;
        }
        DEC_SIG_GET_BIT(bit) //  run length and stop bit
        while (bit == 1)
        {
            x += (1LL << RACC_ZLBITS);
            DEC_SIG_GET_BIT(bit);
        }
        if (x > RACC_BOO)
        { //  infinity norm check
            return 0;
        }
        if (x != 0)
        { //  use sign bit if x != 0
            DEC_SIG_GET_BIT(bit)
            if (bit)
            { //  negative sign
                x = RACC_Q - x;
            }
        }
        sig_z[j] = x;
    }
    *pre_z = z;
    *pre_k = k;
    return l;
}
size_t racc_decode_sig(racc_sig_t *sig, const uint8_t *b)
{
    size_t i, j, k, l, n, b_sz;
    uint8_t bit, z;
    int64_t x;

    //  decode challenge hash
    memcpy(sig->ch, b, RACC_CH_SZ);
    l = RACC_CH_SZ;

    b_sz = RACC_SIG_SZ; //  buffer size
    z = b[l++];
    k = 0;
    //  decode z
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = 0; //  get low bits
            for (n = 0; n < RACC_ZLBITS; n++)
            {
                DEC_SIG_GET_BIT(bit)
                x |= ((int64_t)bit) << n;
            }
            DEC_SIG_GET_BIT(bit) //  run length and stop bit
            while (bit == 1)
            {
                x += (1LL << RACC_ZLBITS);
                DEC_SIG_GET_BIT(bit);
            }
            if (x > RACC_BOO)
            { //  infinity norm check
                return 0;
            }
            if (x != 0)
            { //  use sign bit if x != 0
                DEC_SIG_GET_BIT(bit)
                if (bit)
                { //  negative sign
                    x = RACC_Q - x;
                }
            }
            sig->z[i][j] = x;
        }
    }

    //  decode h
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = 0; //  run length and stop bit
            DEC_SIG_GET_BIT(bit)
            while (bit == 1)
            {
                x++;
                DEC_SIG_GET_BIT(bit)
            }
            if (x > RACC_BOO_H)
            { //  infinity norm check
                return 0;
            }
            if (x != 0)
            {
                DEC_SIG_GET_BIT(bit) //  use sign bit if x != 0
                if (bit)
                {
                    x = -x;
                }
            }
            sig->h[i][j] = x;
        }
    }

    //  check zero padding
    if (k > 0)
    {
        if ((z >> k) != 0) //  fractional bits
            return 0;
        while (l < b_sz)
        { //  zero padding
            if (b[l++] != 0)
                return 0;
        }
    }

    return b_sz;
}
#else
size_t racc_decode_sig(racc_sig_t *sig, const uint8_t *b)
{
    size_t i, j, k, l, n, b_sz;
    uint8_t bit, z;
    int64_t x;

    //  decode challenge hash
    memcpy(sig->ch, b, RACC_CH_SZ);
    l = RACC_CH_SZ;

    b_sz = RACC_SIG_SZ; //  buffer size
    z = b[l++];
    k = 0;

    //  decode h
    for (i = 0; i < RACC_K; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = 0; //  run length and stop bit
            DEC_SIG_GET_BIT(bit)
            while (bit == 1)
            {
                x++;
                DEC_SIG_GET_BIT(bit)
            }
            if (x > RACC_BOO_H)
            { //  infinity norm check
                return 0;
            }
            if (x != 0)
            {
                DEC_SIG_GET_BIT(bit) //  use sign bit if x != 0
                if (bit)
                {
                    x = -x;
                }
            }
            sig->h[i][j] = x;
        }
    }

    //  decode z
    for (i = 0; i < RACC_ELL; i++)
    {
        for (j = 0; j < RACC_N; j++)
        {
            x = 0; //  get low bits
            for (n = 0; n < RACC_ZLBITS; n++)
            {
                DEC_SIG_GET_BIT(bit)
                x |= ((int64_t)bit) << n;
            }
            DEC_SIG_GET_BIT(bit) //  run length and stop bit
            while (bit == 1)
            {
                x += (1LL << RACC_ZLBITS);
                DEC_SIG_GET_BIT(bit);
            }
            if (x > RACC_BOO)
            { //  infinity norm check
                return 0;
            }
            if (x != 0)
            { //  use sign bit if x != 0
                DEC_SIG_GET_BIT(bit)
                if (bit)
                { //  negative sign
                    x = RACC_Q - x;
                }
            }
            sig->z[i][j] = x;
        }
    }

    //  check zero padding
    if (k > 0)
    {
        if ((z >> k) != 0) //  fractional bits
            return 0;
        while (l < b_sz)
        { //  zero padding
            if (b[l++] != 0)
                return 0;
        }
    }

    return b_sz;
}
#endif
#undef DEC_SIG_GET_BIT
