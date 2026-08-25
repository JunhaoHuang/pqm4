# Falcon-512 M4 fixed-point (`m4-fxp`)

Research-only Falcon-512 signing/verification backend using portable four-limb signed Q64.64 arithmetic. It intentionally leaves `m4-ct` unchanged.

- Standard Falcon-512 compact key and signature encodings are retained: public key 897 B, private key 1281 B, and pqm4 signed-message overhead up to 690 B.
- `crypto_sign_keypair()` returns `-1`. Supply an already-valid compact keypair from a provisioning path.
- `crypto_sign()` always decodes and expands the compact key, then calls `Zf(sign_tree)`; the serialized key size is no longer a signing-mode selector. Expansion is redone for every call, so there is no cross-call expanded-key cache.
- Static non-reentrant state is 133,136 B: 53,248 B expansion/work space, a 77,840 B expanded tree, and 2,048 B decoded compact key. Tree signing uses 51,200 B of work space, leaving the final 2,048 B for the shared hash/signature buffer.
- The backend contains no `__int128` type. It is directly cross-compiled with `-mcpu=cortex-m4` and subject to an ARM forbidden-instruction/helper scan.

This is not production-ready. The sampler distribution, whole-algorithm bounds, QEMU testvectors, board performance, stack use, and constant-time behavior require the work listed in `fixed-point-c/docs/falcon-512-m4-fxp-implementation.md`.
