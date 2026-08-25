#include <stddef.h>
#include <string.h>

#include "api.h"
#include "inner.h"
#include "randombytes.h"

/* ==================================================================== */

/*
 * Falcon degree is N = 2^LOGN, where LOGN=9 (for Falcon-512) or 10
 * (for Falcon-1024). We use the advertised public key size to know
 * which degree is used.
 */
#if CRYPTO_PUBLICKEYBYTES == 897
#define LOGN   9
#elif CRYPTO_PUBLICKEYBYTES == 1793
#define LOGN   10
#else
#error Unknown Falcon degree (unexpected public key size)
#endif

#define N   ((size_t)1 << LOGN)
#define NONCELEN   40
#define SEEDLEN    48

/*
 * Always use tree signing. The public pqm4 API still supplies a standard
 * compact Falcon key, which is decoded and expanded in static storage for
 * each signature; the serialized key size is not a signing-mode selector.
 */
#define FXP_SIGN_TREE_TMP_BYTES   (100 * N)
#define FXP_EXPAND_TMP_BYTES      (104 * N)
#define FXP_EXPANDED_KEY_BYTES ((FPR_LEN * ((5 * N) + (LOGN * (N >> 1)))) + FPR_LEN)
typedef char m4_fxp_tree_tail_fits[(FXP_SIGN_TREE_TMP_BYTES + (2 * N) <= FXP_EXPAND_TMP_BYTES) ? 1 : -1];

/*
 * Non-reentrant static storage for compact-key expansion and tree signing.
 * The work union is reused by complete_private(), expand_privkey(),
 * sign_tree(), and verification. The expanded and decoded secret-key
 * regions do not overlap the work region.
 */
static struct {
	union {
		uint8_t b[FXP_EXPAND_TMP_BYTES];
		uint32_t dummy_u32;
		fpr dummy_fp;
	} work;
	union {
		uint8_t b[FXP_EXPANDED_KEY_BYTES];
		uint32_t dummy_u32;
		fpr dummy_fp;
	} expanded;
	uint8_t decoded[4 * N];
} tmp;

/* crypto_sign_keypair: key output parameters; this signing-only backend rejects key generation. */
int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
	(void)pk;
	(void)sk;
	return -1;
}

/* Sign public m into sm/smlen using secret compact sk and internal tree expansion. */
int
crypto_sign(unsigned char *sm, size_t *smlen,
	const unsigned char *m, size_t mlen,
	const unsigned char *sk)
{
	int8_t *f, *g, *F, *G;
	size_t u, v;
	fpr *expanded_key;
	int16_t *sig;
	uint16_t *hm;
	unsigned char seed[SEEDLEN], nonce[NONCELEN];
	unsigned char *esig;
	inner_shake256_context sc;
	size_t sig_len;
	unsigned sav_cw;

	f = (int8_t *)tmp.decoded;
	g = f + N;
	F = g + N;
	G = F + N;
	expanded_key = (fpr *)(void *)tmp.expanded.b;
	/* sign_tree uses exactly work[0..100*N); hm and sig share the tail. */
	sig = (int16_t *)&tmp.work.b[FXP_SIGN_TREE_TMP_BYTES];
	hm = (uint16_t *)sig;
	esig = tmp.work.b;

	/*
	 * Decode the standard compact private key and reconstruct G. This is
	 * deliberately independent of CRYPTO_SECRETKEYBYTES's historical role
	 * as an expanded-key discriminator.
	 */
	if (sk[0] != 0x50 + LOGN) {
		return -1;
	}
	u = 1;
	v = Zf(trim_i8_decode)(f, LOGN, Zf(max_fg_bits)[LOGN],
		sk + u, CRYPTO_SECRETKEYBYTES - u);
	if (v == 0) {
		return -1;
	}
	u += v;
	v = Zf(trim_i8_decode)(g, LOGN, Zf(max_fg_bits)[LOGN],
		sk + u, CRYPTO_SECRETKEYBYTES - u);
	if (v == 0) {
		return -1;
	}
	u += v;
	v = Zf(trim_i8_decode)(F, LOGN, Zf(max_FG_bits)[LOGN],
		sk + u, CRYPTO_SECRETKEYBYTES - u);
	if (v == 0) {
		return -1;
	}
	u += v;
	if (u != CRYPTO_SECRETKEYBYTES) {
		return -1;
	}
	if (!Zf(complete_private)(G, f, g, F, LOGN, tmp.work.b)) {
		return -1;
	}
	Zf(expand_privkey)(expanded_key, f, g, F, G, LOGN, tmp.work.b);

	/*
	 * Create a random nonce and hash nonce + message into a Falcon point.
	 */
	randombytes(nonce, NONCELEN);
	inner_shake256_init(&sc);
	inner_shake256_inject(&sc, nonce, NONCELEN);
	inner_shake256_inject(&sc, m, mlen);
	inner_shake256_flip(&sc);
	Zf(hash_to_point_vartime)(&sc, hm, LOGN);

	/* Initialize the signing RNG. */
	randombytes(seed, SEEDLEN);
	inner_shake256_init(&sc);
	inner_shake256_inject(&sc, seed, SEEDLEN);
	inner_shake256_flip(&sc);

	/* Sign with the just-expanded tree; the scratch and key are disjoint. */
	sav_cw = set_fpu_cw(2);
	Zf(sign_tree)(sig, &sc, expanded_key, hm, LOGN, tmp.work.b);
	set_fpu_cw(sav_cw);

	/*
	 * Encode the signature and bundle it with the message. Format is:
	 *   signature length     2 bytes, big-endian
	 *   nonce                40 bytes
	 *   message              mlen bytes
	 *   signature            slen bytes
	 */
	esig[0] = 0x20 + LOGN;
	sig_len = Zf(comp_encode)(esig + 1, CRYPTO_BYTES - 1, sig, LOGN);
	if (sig_len == 0) {
		return -1;
	}
	sig_len ++;
	memmove(sm + 2 + NONCELEN, m, mlen);
	sm[0] = (unsigned char)(sig_len >> 8);
	sm[1] = (unsigned char)sig_len;
	memcpy(sm + 2, nonce, NONCELEN);
	memcpy(sm + 2 + NONCELEN + mlen, esig, sig_len);
	*smlen = 2 + NONCELEN + mlen + sig_len;
	return 0;
}

/* Verify public sm with public pk, returning its message through m/mlen. */
int
crypto_sign_open(unsigned char *m, size_t *mlen,
	const unsigned char *sm, size_t smlen,
	const unsigned char *pk)
{
	uint16_t *h, *hm;
	int16_t *sig;
	const unsigned char *esig;
	inner_shake256_context sc;
	size_t sig_len, msg_len;

	h = (uint16_t *)&tmp.work.b[2 * N];
	hm = h + N;
	sig = (int16_t *)(hm + N);

	/*
	 * Decode public key.
	 */
	if (pk[0] != 0x00 + LOGN) {
		return -1;
	}
	if (Zf(modq_decode)(h, LOGN, pk + 1, CRYPTO_PUBLICKEYBYTES - 1)
		!= CRYPTO_PUBLICKEYBYTES - 1)
	{
		return -1;
	}
	Zf(to_ntt_monty)(h, LOGN);

	/*
	 * Find nonce, signature, message length.
	 */
	if (smlen < 2 + NONCELEN) {
		return -1;
	}
	sig_len = ((size_t)sm[0] << 8) | (size_t)sm[1];
	if (sig_len > (smlen - 2 - NONCELEN)) {
		return -1;
	}
	msg_len = smlen - 2 - NONCELEN - sig_len;

	/*
	 * Decode signature.
	 */
	esig = sm + 2 + NONCELEN + msg_len;
	if (sig_len < 1 || esig[0] != 0x20 + LOGN) {
		return -1;
	}
	if (Zf(comp_decode)(sig, LOGN,
		esig + 1, sig_len - 1) != sig_len - 1)
	{
		return -1;
	}

	/*
	 * Hash nonce + message into a vector.
	 */
	inner_shake256_init(&sc);
	inner_shake256_inject(&sc, sm + 2, NONCELEN + msg_len);
	inner_shake256_flip(&sc);
	Zf(hash_to_point_vartime)(&sc, hm, LOGN);

	/*
	 * Verify signature.
	 */
	if (!Zf(verify_raw)(hm, sig, h, LOGN, tmp.work.b)) {
		return -1;
	}

	/*
	 * Return plaintext.
	 */
	memmove(m, sm + 2 + NONCELEN, msg_len);
	*mlen = msg_len;
	return 0;
}
