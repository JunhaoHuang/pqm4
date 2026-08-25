/*
 * Portable Q64.64 non-inline operations.
 *
 * This implementation contains the fixed-iteration reciprocal, inverse
 * square-root, and exponential polynomial needed by the Falcon sampler.
 * It is selected only by FALCON_FXP_PORTABLE; the historical host oracle
 * remains below as a host-side differential oracle.
 */

/* Return |x|; input: Q64.64 x; output: unsigned-magnitude representation. */
static fpr
fpr_abs(fpr x)
{
	return fpr_ct_select(x, fpr_neg(x), (uint32_t)-(x.w[3] >> 31));
}

/* Locate the highest non-zero bit with a four-word fixed schedule; input: nonzero magnitude; output: bit index. */
static uint32_t
fpr_msb_index(fpr x)
{
	uint32_t r = 0;
	unsigned u;
	for (u = 0; u < 4; u ++) {
		uint32_t w = x.w[u];
		uint32_t c = 31U - (uint32_t)__builtin_clz(w | 1U);
		r = fpr_ct_select_u32(r, (uint32_t)(u << 5) + c, fpr_ct_nz(w));
	}
	return r;
}

/* Compute a quotient through five Newton reciprocal steps; inputs: numerator/denominator; output: rounded quotient. */
fpr
fpr_div(fpr x, fpr y)
{
	fpr ax = fpr_abs(x);
	fpr ay = fpr_abs(y);
	uint32_t sign = (x.w[3] ^ y.w[3]) >> 31;
	int32_t shift = 63 - (int32_t)fpr_msb_index(ay);
	fpr b = fpr_shift_signed(ay, shift);
	fpr a = fpr_shift_signed(ax, shift);
	fpr r = fpr_sub(c48_17, fpr_mul(b, c32_17));
	unsigned u;
	for (u = 0; u < 5; u ++) {
		r = fpr_add(r, fpr_mul(r, fpr_sub(fpr_one, fpr_mul(b, r))));
	}
	r = fpr_mul(a, r);
	return fpr_ct_select(r, fpr_neg(r), (uint32_t)-sign);
}

/* Compute sqrt through eight inverse-square-root Newton steps; input: positive Q64.64 x; output: sqrt(x). */
fpr
fpr_sqrt(fpr x)
{
	fpr r = { UINT128(0, 0x028f5c28f5c28f5dULL) };
	fpr three = { UINT128(3, 0) };
	unsigned u;
	for (u = 0; u < 8; u ++) {
		r = fpr_mul(r, fpr_div2e(fpr_sub(three, fpr_mul(x, fpr_sqr(r))), 1));
	}
	return fpr_mul(r, x);
}

/* Return high 64 bits of a 64-by-64 product; inputs: unsigned words; output: floor(a*b/2^64). */
static uint64_t
fpr_mul_hi_u64(uint64_t a, uint64_t b)
{
	uint64_t p00 = (uint64_t)(uint32_t)a * (uint32_t)b;
	uint64_t p01 = (uint64_t)(uint32_t)a * (uint32_t)(b >> 32);
	uint64_t p10 = (uint64_t)(uint32_t)(a >> 32) * (uint32_t)b;
	uint64_t p11 = (uint64_t)(uint32_t)(a >> 32) * (uint32_t)(b >> 32);
	uint64_t t = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
	uint64_t u = (p01 >> 32) + (p10 >> 32) + (uint32_t)p11 + (t >> 32);
	return (((p11 >> 32) + (u >> 32)) << 32) | (uint32_t)u;
}

/* Evaluate the fixed FACCT exponential polynomial; inputs: x/ccs in Q64.64; output: 2^64-scaled result. */
uint64_t
fpr_expm_p63(fpr x, fpr ccs)
{
	static const uint64_t C[13] = {
		0x00000004741183A3ULL, 0x00000036548CFC06ULL,
		0x0000024FDCBF140AULL, 0x0000171D939DE045ULL,
		0x0000D00CF58F6F84ULL, 0x000680681CF796E3ULL,
		0x002D82D8305B0FEAULL, 0x011111110E066FD0ULL,
		0x0555555555070F00ULL, 0x155555555581FF00ULL,
		0x400000000002B400ULL, 0x7FFFFFFFFFFF4800ULL,
		0x8000000000000000ULL
	};
	uint64_t z = fpr_scale_p63(x);
	uint64_t r = C[0];
	unsigned u;
	for (u = 1; u < 13; u ++) {
		r = C[u] - fpr_mul_hi_u64(z, r);
	}
	return fpr_mul_hi_u64(fpr_scale_p63(ccs), r);
}
