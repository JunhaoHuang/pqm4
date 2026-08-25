/*
 * Portable Q64.64 arithmetic for the fixed-point Falcon backend.
 *
 * Values are two's-complement integers scaled by 2^64 and held in four
 * little-endian uint32_t limbs.  The routines below are the portable oracle
 * for the Cortex-M4 backend: they use a fixed operation schedule, have no
 * native division or remainder, and keep secret-dependent choices masked.
 */
#ifndef FALCON_FPR_LIMB_H__
#define FALCON_FPR_LIMB_H__

#include <stdint.h>

typedef struct {
	uint64_t v;
} fpr64;

typedef struct {
	uint32_t w[4];
} fpr;

/* Convert two 64-bit words written high/low into little-endian limbs. */
#define UINT128(hi, lo) { \
	(uint32_t)(uint64_t)(lo), (uint32_t)((uint64_t)(lo) >> 32), \
	(uint32_t)(uint64_t)(hi), (uint32_t)((uint64_t)(hi) >> 32) }

/* Public fixed-point constants, exactly copied from the Q64.64 prototype. */
static const fpr fpr_q = { UINT128(0x0000000000003001ULL, 0) };
static const fpr fpr_inverse_of_q = { UINT128(0, 0x00055538e425e9e1ULL) };
static const fpr fpr_inv_2sqrsigma0 = { UINT128(0, 0x269f178307778416ULL) };
static const fpr fpr_log2 = { UINT128(0, 0xb17217f7d1cf79acULL) };
static const fpr fpr_inv_log2 = { UINT128(1, 0x71547652b82fe178ULL) };
static const fpr fpr_bnorm_max = { UINT128(0x00000000000041b6ULL, 0x697f62b6ae7d566dULL) };
static const fpr fpr_zero = { UINT128(0, 0) };
static const fpr fpr_one = { UINT128(1, 0) };
static const fpr fpr_two = { UINT128(2, 0) };
static const fpr fpr_onehalf = { UINT128(0, 0x8000000000000000ULL) };
static const fpr fpr_invsqrt2 = { UINT128(0, 0xb504f333f9de6485ULL) };
static const fpr fpr_invsqrt8 = { UINT128(0, 0x5a827999fcef3243ULL) };
static const fpr fpr_ptwo31 = { UINT128(0x0000000080000000ULL, 0) };
static const fpr fpr_ptwo31m1 = { UINT128(0x000000007fffffffULL, 0) };
static const fpr fpr_mtwo31m1 = { UINT128(0xffffffff80000001ULL, 0) };
static const fpr fpr_ptwo63m1 = { UINT128(0x7fffffffffffffffULL, 0) };
static const fpr fpr_mtwo63m1 = { UINT128(0x8000000000000001ULL, 0) };
static const fpr fpr_ptwo63 = { UINT128(0x8000000000000000ULL, 0) };
static const fpr c48_17 = { UINT128(2, 0xd2d2d2d2d2d2db4eULL) };
static const fpr c32_17 = { UINT128(1, 0xe1e1e1e1e1e1e2bbULL) };

static const fpr fpr_inv_sigma[] = {
	{ UINT128(0, 0) },
	{ UINT128(0, 0x01c48eb7e24169a5ULL) },
	{ UINT128(0, 0x01be50a548caed8fULL) },
	{ UINT128(0, 0x01b852ee09e762bdULL) },
	{ UINT128(0, 0x01afc5ed3cada35cULL) },
	{ UINT128(0, 0x01a7b3b0976b3ed1ULL) },
	{ UINT128(0, 0x01a011282ca9c97eULL) },
	{ UINT128(0, 0x0198d49ce5f27362ULL) },
	{ UINT128(0, 0x0191f57c56ed9ee1ULL) },
	{ UINT128(0, 0x018b6c2de64c7ca6ULL) },
	{ UINT128(0, 0x018531ef6311ae2aULL) }
};
static const fpr fpr_q_div_sigma2[] = {
	{ UINT128(0, 0) },
	{ UINT128(0, 0x9604adb3afcb03eaULL) },
	{ UINT128(0, 0x91e87edc5a6f227bULL) },
	{ UINT128(0, 0x8e046dd31072c9aeULL) },
	{ UINT128(0, 0x888e1ad4cc14edceULL) },
	{ UINT128(0, 0x837f5c353713b75eULL) },
	{ UINT128(0, 0x7ecd18ea2c79a66dULL) },
	{ UINT128(0, 0x7a6dbfc79e08a461ULL) },
	{ UINT128(0, 0x7659062fe5c91999ULL) },
	{ UINT128(0, 0x7287b36828d730fcULL) },
	{ UINT128(0, 0x6ef375cce7a90066ULL) }
};
static const fpr fpr_sigma_min[] = {
	{ UINT128(0, 0) },
	{ UINT128(1, 0x1dd380644568b615ULL) },
	{ UINT128(1, 0x21d2edcad862674aULL) },
	{ UINT128(1, 0x25c46e1aa7c7a112ULL) },
	{ UINT128(1, 0x2b95c574afb249abULL) },
	{ UINT128(1, 0x314abc7fe22b5eb6ULL) },
	{ UINT128(1, 0x36e4e3475d7c34fbULL) },
	{ UINT128(1, 0x3c65a66a1c223924ULL) },
	{ UINT128(1, 0x41ce5358cb39fb5aULL) },
	{ UINT128(1, 0x47201bf1f7a74a9cULL) },
	{ UINT128(1, 0x4c5c19990c763c10ULL) }
};

/* Return all ones iff a == b; inputs: public/secret words; output: mask. */
static inline uint32_t
fpr_ct_eq(uint32_t a, uint32_t b)
{
	uint32_t x = a ^ b;
	x = (x | (uint32_t)-x) >> 31;
	return x - 1;
}

/* Return all ones iff x is non-zero; input: public/secret word; output: mask. */
static inline uint32_t
fpr_ct_nz(uint32_t x)
{
	return (uint32_t)-((x | (uint32_t)-x) >> 31);
}

/* Select b when mask is all ones, otherwise a; inputs: values/mask; output: word. */
static inline uint32_t
fpr_ct_select_u32(uint32_t a, uint32_t b, uint32_t mask)
{
	return a ^ (mask & (a ^ b));
}

/* Select b when mask is all ones, otherwise a; inputs: values/mask; output: fpr. */
static inline fpr
fpr_ct_select(fpr a, fpr b, uint32_t mask)
{
	fpr r;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		r.w[u] = fpr_ct_select_u32(a.w[u], b.w[u], mask);
	}
	return r;
}

/* Read an unsigned limb with a masked full scan; inputs: value/index; output: limb or zero. */
static inline uint32_t
fpr_word(fpr x, uint32_t index)
{
	uint32_t r = 0;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		r |= x.w[u] & fpr_ct_eq(index, u);
	}
	return r;
}

/* Read a sign-extended limb with a masked full scan; inputs: value/index; output: limb. */
static inline uint32_t
fpr_word_signed(fpr x, uint32_t index)
{
	uint32_t in_range = fpr_ct_eq(index, 0) | fpr_ct_eq(index, 1)
		| fpr_ct_eq(index, 2) | fpr_ct_eq(index, 3);
	uint32_t sign = (uint32_t)-(x.w[3] >> 31);
	return fpr_ct_select_u32(sign, fpr_word(x, index), in_range);
}

/* Shift left by 0..127 bits; input: Q64.64 value/count; output: shifted value. */
static inline fpr
fpr_shl(fpr x, uint32_t n)
{
	fpr r;
	uint32_t q = n >> 5;
	uint32_t s = n & 31;
	uint32_t nz = fpr_ct_nz(s);
	unsigned u;
	for (u = 0; u < 4; u ++) {
		uint32_t lo = fpr_word(x, (uint32_t)u - q);
		uint32_t hi = fpr_word(x, (uint32_t)u - q - 1);
		r.w[u] = (lo << s) | ((hi >> ((32 - s) & 31)) & nz);
	}
	return r;
}

/* Arithmetic shift right by 0..127 bits; input: Q64.64 value/count; output: shifted value. */
static inline fpr
fpr_asr(fpr x, uint32_t n)
{
	fpr r;
	uint32_t q = n >> 5;
	uint32_t s = n & 31;
	uint32_t nz = fpr_ct_nz(s);
	unsigned u;
	for (u = 0; u < 4; u ++) {
		uint32_t lo = fpr_word_signed(x, (uint32_t)u + q);
		uint32_t hi = fpr_word_signed(x, (uint32_t)u + q + 1);
		r.w[u] = (lo >> s) | ((hi << ((32 - s) & 31)) & nz);
	}
	return r;
}

/* Forward declaration: fpr_shift_signed uses the modular adder below. */
static inline fpr fpr_add(fpr x, fpr y);

/* Shift by a signed count; input: value/count; output: left or rounded arithmetic-right shift. */
static inline fpr
fpr_shift_signed(fpr x, int32_t n)
{
	uint32_t neg = (uint32_t)n >> 31;
	uint32_t l = (uint32_t)n & (uint32_t)-(neg ^ 1U);
	uint32_t r = (uint32_t)-n & (uint32_t)-neg;
	fpr round = { { 0, 0, 0, 0 } };
	fpr right;
	uint32_t bit = (r - 1U) & 127U;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		round.w[u] = (1U << (bit & 31)) & fpr_ct_eq(bit >> 5, u) & fpr_ct_nz(r);
	}
	right = fpr_asr(fpr_add(x, round), r);
	return fpr_ct_select(fpr_shl(x, l), right, (uint32_t)-neg);
}

/* Add two Q64.64 values; inputs: x/y; output: x+y modulo 2^128. */
static inline fpr
fpr_add(fpr x, fpr y)
{
	fpr r;
	uint32_t carry = 0;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		uint32_t s = x.w[u] + y.w[u];
		uint32_t c1 = (uint32_t)-(s < x.w[u]);
		uint32_t t = s + carry;
		uint32_t c2 = (uint32_t)-(t < s);
		r.w[u] = t;
		carry = (c1 | c2) & 1U;
	}
	return r;
}

/* Negate a Q64.64 value; input: x; output: -x modulo 2^128. */
static inline fpr
fpr_neg(fpr x)
{
	fpr r;
	uint32_t carry = 1;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		uint32_t t = ~x.w[u] + carry;
		carry &= (uint32_t)(t == 0);
		r.w[u] = t;
	}
	return r;
}

/* Subtract two Q64.64 values; inputs: x/y; output: x-y modulo 2^128. */
static inline fpr
fpr_sub(fpr x, fpr y)
{
	return fpr_add(x, fpr_neg(y));
}

/* Convert an integer; input: signed i; output: i in Q64.64. */
static inline fpr
fpr_of(int64_t i)
{
	uint64_t u = (uint64_t)i;
	uint32_t s = (uint32_t)-(uint32_t)(i < 0);
	fpr r = { { (uint32_t)u, (uint32_t)(u >> 32), s, s } };
	return fpr_shl(r, 64);
}

/* Convert and scale an integer; input: i and public bit count; output: i*2^sc in Q64.64. */
static inline fpr
fpr_scaled(int64_t i, int sc)
{
	return fpr_shl(fpr_of(i), (uint32_t)sc);
}

/* Convert a Q32.32 raw integer; input: i; output: corresponding Q64.64 value. */
static inline fpr
fpr_of_scaled(int64_t i)
{
	uint64_t u = (uint64_t)i;
	uint32_t s = (uint32_t)-(uint32_t)(i < 0);
	/* Match the historical fpr64-to-fpr embedding bit for bit. */
	fpr r = { { (uint32_t)u, 0, (uint32_t)(u >> 32), s } };
	return r;
}

/* Convert Q64.64 to Q32.32; input: x; output: truncated raw Q32.32 value. */
static inline fpr64
fpr_to_fpr64(fpr x)
{
	fpr64 r = { (uint64_t)x.w[1] | ((uint64_t)x.w[2] << 32) };
	return r;
}

/* Round to nearest integer (ties toward +infinity); input: bounded x; output: signed integer. */
static inline int64_t
fpr_rint(fpr x)
{
	fpr half = { { 0, 0x80000000U, 0, 0 } };
	x = fpr_add(x, half);
	return (int64_t)((uint64_t)x.w[2] | ((uint64_t)x.w[3] << 32));
}

/* Truncate/floor a Q64.64 value; input: bounded x; output: signed integer. */
static inline int64_t
fpr_trunc(fpr x)
{
	return (int64_t)((uint64_t)x.w[2] | ((uint64_t)x.w[3] << 32));
}

/* Convert the integral part to Q64.64; input: x; output: its integral part. */
static inline fpr
fpr_trunc_fpr(fpr x)
{
	return fpr_of(fpr_trunc(x));
}

/* Return the fractional raw bits; input: x; output: signed low 64 bits. */
static inline int64_t
fpr_frac(fpr x)
{
	return (int64_t)((uint64_t)x.w[0] | ((uint64_t)x.w[1] << 32));
}

/* Return the fractional Q64.64 value; input: x; output: x modulo one. */
static inline fpr
fpr_frac_fpr(fpr x)
{
	fpr r = { { x.w[0], x.w[1], 0, 0 } };
	return r;
}

/* Return the floor; input: bounded x; output: signed integer. */
static inline int64_t
fpr_floor(fpr x)
{
	return fpr_trunc(x);
}

/* Divide by 2^n with prototype-compatible rounding; input: x/public n; output: rounded x/2^n. */
static inline fpr
fpr_div2e(fpr x, unsigned n)
{
	return fpr_shift_signed(x, -(int32_t)n);
}

/* Halve a Q64.64 value; input: x; output: rounded x/2. */
static inline fpr
fpr_half(fpr x)
{
	return fpr_div2e(x, 1);
}

/* Double a Q64.64 value; input: x; output: 2*x modulo 2^128. */
static inline fpr
fpr_double(fpr x)
{
	return fpr_shl(x, 1);
}

/* Multiply by 2^n; input: x/public n; output: scaled value modulo 2^128. */
static inline fpr
fpr_mul2e(fpr x, unsigned n)
{
	return fpr_shl(x, n);
}

/* Multiply two Q64.64 values using a fixed 4-by-4 limb schedule; inputs: x/y; output: rounded product. */
static inline fpr
fpr_mul(fpr x, fpr y)
{
	uint32_t p[8] = { 0 };
	unsigned i, j;
	for (i = 0; i < 4; i ++) {
		uint32_t carry = 0;
		for (j = 0; j < 4; j ++) {
			uint64_t t = (uint64_t)x.w[i] * y.w[j] + p[i + j] + carry;
			p[i + j] = (uint32_t)t;
			carry = (uint32_t)(t >> 32);
		}
		for (j = i + 4; j < 8; j ++) {
			uint64_t t = (uint64_t)p[j] + carry;
			p[j] = (uint32_t)t;
			carry = (uint32_t)(t >> 32);
		}
	}
	{
		uint32_t before = p[1];
		uint32_t carry;
		p[1] = before + 0x80000000U;
		carry = (uint32_t)(p[1] < before);
		for (i = 2; i < 8; i ++) {
			before = p[i];
			p[i] = before + carry;
			carry = (uint32_t)(p[i] < before);
		}
	}
	{
		fpr r = { { p[2], p[3], p[4], p[5] } };
		/* Convert the unsigned limb product into the signed two's-complement product. */
		r = fpr_ct_select(r, fpr_sub(r, fpr_shl(x, 64)), (uint32_t)-(y.w[3] >> 31));
		r = fpr_ct_select(r, fpr_sub(r, fpr_shl(y, 64)), (uint32_t)-(x.w[3] >> 31));
		return r;
	}
}

/* Square a Q64.64 value; input: x; output: rounded x*x. */
static inline fpr
fpr_sqr(fpr x)
{
	return fpr_mul(x, x);
}

/* Compare signed Q64.64 values; inputs: x/y; output: 1 iff x<y. */
static inline int
fpr_lt(fpr x, fpr y)
{
	uint32_t sx = x.w[3] >> 31;
	uint32_t sy = y.w[3] >> 31;
	uint32_t sd = fpr_sub(x, y).w[3] >> 31;
	return (int)fpr_ct_select_u32(sd, sx, (uint32_t)-(sx ^ sy));
}

/* Scale a non-negative value below one for expm; input: x; output: x*2^64 or zero on overflow. */
static inline uint64_t
fpr_scale_p63(fpr x)
{
	uint64_t r = (uint64_t)x.w[0] | ((uint64_t)x.w[1] << 32);
	uint64_t m = (uint64_t)-(uint64_t)((x.w[2] & 1U) ^ 1U);
	return r & m;
}

/* Divide/invert and square-root are implemented in fpr.c to keep the portable core auditable. */
#define fpr_div Zf(fpr_div)
#define fpr_sqrt Zf(fpr_sqrt)
#define fpr_expm_p63 Zf(fpr_expm_p63)
fpr fpr_div(fpr x, fpr y);
static inline fpr fpr_inv(fpr x) { return fpr_div(fpr_one, x); }
fpr fpr_sqrt(fpr x);
uint64_t fpr_expm_p63(fpr x, fpr ccs);
#define fpr_gm_tab Zf(fpr_gm_tab)
#define fpr_p2_tab Zf(fpr_p2_tab)
extern const fpr fpr_gm_tab[];
extern const fpr fpr_p2_tab[];

#endif
