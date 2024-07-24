1. modular arithmetic
2. NTT optimizations
3. memory optimizations for keygen, sign and verify 
   1. raccoon128:k,l,d=5,4,d; raccoon192: k,l,d=7,5,d; raccoon256: k,l,d=9,7,d; d=1,2,4,8,16,32.
   2. pk: 4k; sk: 4ld+4k; sig: 4(k+l);
   3. `racc_core_keygen`: 4l+4d
   4. `racc_core_sign` too much memory. 4kld+4ld+4d+4k+l+3*4
   5. `racc_core_verify`: 4*4+4k+4l
4. racc_core_keygen:
   1. the memory of `pk, sk` and `encode_pk, encode_sk` in `keygen` can be merged, reused or in-place.
   2. the memory of `sk` is determined by `RACC_D`. Not so suitable for large `RACC_D`; For `RACC_D=32`, the `sk` size is `532KiB, 668KiB, 932KiB`, respectively, which is definitely not suitable for embedding device.
   3. `encode_sk` has a redundant `encode_pk`, can be reduced.
   4. expand_aij on-the-fly ai; reduce 4(l-2)=12k stack usage for raccoon-128-2.
   5. add_rep_noise, racc_decode, racc_refresh: on-the-fly mt, vi; add_rep_noise难搞，涉及randombytes顺序问题
5. racc_core_sign:
   1. matrix `ma` is used twice. regenerate or large matrix? $4kl=80,140,252$
   2. `sk`: $4ld=532, 668, 932$ for $d=32$
   3. matrix `mr` $l\times D$ matrix; $l$-degree polynomial vector each with D shares. It is used twice too. $4ld=532, 668, 932$ for $d=32$
   4. matrix `mw` 1 poly with D shares; $4d=32$ for $d=32$
   5. `vw` $k$-degree polynomial vector; $4k$
   6. `vz` $l$-degree polynomial vector; $4l$
   7. `u` and `c_poly` use small poly mul?; $4$
6. racc_core_verify:
   1. `aij` has been optimized.
   2. `c_poly` use small poly mul? : It has RACC_W $\pm 1$: 19, 31, 44
   3. `vw` $k$-degree polynomial vector
   4. `vz` $l$-degree polynomial vector
   5. `t` and `u`

**The goal of this project is to reduce the memory as many as possible so that we can perform the 32-share masking, while also improve the performance compared with the reference C implementation using the assembly implementation.**
1. Dilihtium impl stream `A, y` is not enough.
2. In order to enable high-order masking, we also need to stream the variale with `d`-shares. `sk` with 32-share can be upto **KiB.
- 修改RACC_D相关的函数，使得RACC_D相关的变量on-the-fly。
  - zero_encoding: 这个得D个多项式一起参与，如何修改使其on-the-fly？
  - add_rep_noise: done: with a buf to precompute the randombytes.
  - encode_sk
  - decode_sk
3. racc_core_sign
- on-the-fly matrix `A`
- on-the-fly (de-)serialization of `sig` and `sk`
- on-the-fly `vw, rz`
- 
**In term of Performance**
1. `round_shift_r` assembly
2. `poly_split` assembly
3. 