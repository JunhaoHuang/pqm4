KEM_SCHEMES := \
	crypto_kem/kyber1024/m4fstack \
	crypto_kem/kyber1024/m4fspeed \
	crypto_kem/bikel3/m4f \
	crypto_kem/bikel1/m4f \
	crypto_kem/kyber768/m4fstack \
	crypto_kem/kyber768/m4fspeed \
	crypto_kem/kyber512/m4fstack \
	crypto_kem/kyber512/m4fspeed \
	mupq/crypto_kem/bikel3/opt \
	mupq/crypto_kem/bikel1/opt \
	mupq/pqclean/crypto_kem/mceliece348864/clean \
	mupq/pqclean/crypto_kem/kyber1024/clean \
	mupq/pqclean/crypto_kem/mceliece8192128/clean \
	mupq/pqclean/crypto_kem/mceliece8192128f/clean \
	mupq/pqclean/crypto_kem/mceliece6960119f/clean \
	mupq/pqclean/crypto_kem/mceliece6688128/clean \
	mupq/pqclean/crypto_kem/hqc-rmrs-192/clean \
	mupq/pqclean/crypto_kem/kyber1024-90s/clean \
	mupq/pqclean/crypto_kem/mceliece460896/clean \
	mupq/pqclean/crypto_kem/hqc-rmrs-256/clean \
	mupq/pqclean/crypto_kem/mceliece460896f/clean \
	mupq/pqclean/crypto_kem/mceliece6960119/clean \
	mupq/pqclean/crypto_kem/hqc-rmrs-128/clean \
	mupq/pqclean/crypto_kem/kyber512-90s/clean \
	mupq/pqclean/crypto_kem/mceliece6688128f/clean \
	mupq/pqclean/crypto_kem/kyber768-90s/clean \
	mupq/pqclean/crypto_kem/kyber768/clean \
	mupq/pqclean/crypto_kem/kyber512/clean \
	mupq/pqclean/crypto_kem/mceliece348864f/clean

SIGN_SCHEMES := \
	crypto_sign/raccoon-192/m4 \
	crypto_sign/raccoon-192/ref \
	crypto_sign/perk-128-short-5/m4 \
	crypto_sign/perk-256-short-3/m4 \
	crypto_sign/perk-128-fast-5/m4 \
	crypto_sign/perk-256-fast-5/m4 \
	crypto_sign/masked_dilithium3/m4 \
	crypto_sign/falcon-1024/m4-ct \
	crypto_sign/raccoon-128/m4 \
	crypto_sign/raccoon-128/ref \
	crypto_sign/perk-192-short-3/m4 \
	crypto_sign/perk-256-fast-3/m4 \
	crypto_sign/ov-Ip/m4f \
	crypto_sign/mayo3/m4f \
	crypto_sign/ov-Ip-pkc-skc/m4fstack \
	crypto_sign/ov-Ip-pkc-skc/m4fspeed \
	crypto_sign/perk-128-short-3/m4 \
	crypto_sign/haetae3/m4f \
	crypto_sign/perk-192-fast-5/m4 \
	crypto_sign/dilithium2/m4f \
	crypto_sign/perk-256-short-5/m4 \
	crypto_sign/haetae2/m4f \
	crypto_sign/falcon-512/m4-ct \
	crypto_sign/mayo2/m4f \
	crypto_sign/dilithium3/m4f \
	crypto_sign/haetae5/m4f \
	crypto_sign/ov-Ip-pkc/m4fstack \
	crypto_sign/ov-Ip-pkc/m4fspeed \
	crypto_sign/dilithium5/m4f \
	crypto_sign/falcon-512-tree/m4-ct \
	crypto_sign/mayo1/m4f \
	crypto_sign/perk-192-fast-3/m4 \
	crypto_sign/perk-128-fast-3/m4 \
	crypto_sign/raccoon-256/m4 \
	crypto_sign/raccoon-256/ref \
	crypto_sign/perk-192-short-5/m4 \
	mupq/crypto_sign/falcon-1024/opt-ct \
	mupq/crypto_sign/falcon-1024/opt-leaktime \
	mupq/crypto_sign/falcon-512/opt-ct \
	mupq/crypto_sign/falcon-512/opt-leaktime \
	mupq/crypto_sign/falcon-512-tree/opt-ct \
	mupq/crypto_sign/falcon-512-tree/opt-leaktime \
	mupq/crypto_sign/falcon-1024-tree/opt-ct \
	mupq/crypto_sign/falcon-1024-tree/opt-leaktime \
	mupq/pqclean/crypto_sign/sphincs-haraka-192f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-256s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-128s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-128f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-192f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-128f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-128s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-192s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-256s-simple/clean \
	mupq/pqclean/crypto_sign/dilithium3aes/clean \
	mupq/pqclean/crypto_sign/dilithium2aes/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-256f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-128s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-192f-robust/clean \
	mupq/pqclean/crypto_sign/falcon-1024/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-192f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-192s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-256s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-128f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-128f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-192s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-192s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-192s-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-256s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-256f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-256f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-128s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-256f-robust/clean \
	mupq/pqclean/crypto_sign/dilithium2/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-256s-robust/clean \
	mupq/pqclean/crypto_sign/falcon-512/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-128f-robust/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-192f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-128f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-128s-robust/clean \
	mupq/pqclean/crypto_sign/dilithium3/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-192s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-256s-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-haraka-128s-robust/clean \
	mupq/pqclean/crypto_sign/dilithium5aes/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-256f-simple/clean \
	mupq/pqclean/crypto_sign/dilithium5/clean \
	mupq/pqclean/crypto_sign/sphincs-shake256-192f-simple/clean \
	mupq/pqclean/crypto_sign/sphincs-sha256-256f-simple/clean