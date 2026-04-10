## Hashing profile
1. keygen: 18%, enc: 32%, dec: 10%

## profile on Intel CPU: 
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  us/call  us/call  name    
 26.10      1.36     1.36 12800512     0.11     0.11  base_inv
 16.12      2.20     0.84   100001     8.40    12.23  PKE_Decrypt
 15.16      2.99     0.79  2500056     0.32     0.32  KeccakF1600_StatePermute
  9.21      3.47     0.48   400008     1.20     1.20  mq_poly_ntt
  6.33      3.80     0.33   300004     1.10     1.10  mq_poly_intt
  4.03      4.01     0.21   300004     0.70     0.83  mq_poly_pointwise_mul
  3.07      4.17     0.16   100004     1.60     3.20  FastInversion
  2.88      4.32     0.15   100004     1.50     1.50  mq_poly_ntt_mq
  2.50      4.45     0.13   200003     0.65     0.65  decode_pk
  2.30      4.57     0.12   100004     1.20     1.90  mq_poly_pointwise_mul_mq
  2.11      4.68     0.11   600024     0.18     0.18  Mul_in_R2_n
  1.92      4.78     0.10   200003     0.50     1.69  ternary_sample_se
  1.34      4.85     0.07  6400256     0.01     0.01  base_mul_mq
  1.34      4.92     0.07   100001     0.70     0.70  decode_ct
  0.96      4.97     0.05   600024     0.08     0.08  Mulf_in_R2_N2
  0.96      5.02     0.05   200003     0.25     5.59  PKE_Encrypt
  0.77      5.06     0.04 19200256     0.00     0.00  base_mul
  0.77      5.10     0.04   200003     0.20     0.20  encode_ct
  0.58      5.13     0.03   100004     0.30     1.81  ternary_sample_fgk
  0.19      5.14     0.01   400009     0.02     0.66  sha3_256
  0.19      5.15     0.01   300007     0.03     1.19  shake256_squeezeblocks
  0.19      5.16     0.01   200008     0.05     0.05  check_poly_inv_Zq
  0.19      5.17     0.01   200003     0.05     0.37  sha3_512
  0.19      5.18     0.01   100004     0.10    23.72  PKE_KeyGen
  0.19      5.19     0.01   100004     0.10     0.10  encode_f
  0.19      5.20     0.01   100004     0.10     0.10  encode_pk
  0.19      5.21     0.01   100001     0.10     0.10  decode_f
  0.00      5.21     0.00   300007     0.00     0.32  shake256_absorb_once
  0.00      5.21     0.00   200003     0.00     0.00  poly_round
  0.00      5.21     0.00   100004     0.00    24.58  KEM_KeyGen
  0.00      5.21     0.00   100004     0.00     0.00  check_poly_inv_Z2
  0.00      5.21     0.00   100004     0.00    13.60  mq_poly_inv_ntt
  0.00      5.21     0.00   100004     0.00     0.32  shake256_squeeze
  0.00      5.21     0.00   100002     0.00     7.92  KEM_Enc
  0.00      5.21     0.00   100001     0.00    19.60  KEM_Dec

## TODO optimizations
1. hash for reference and m4 should be different? to highlight the Keccak optimization by our paper.
2. 1024 version of KEM is wrong. kem_dec: pke_decrypt cannot end. try to not use 512 version of KEM, and self-create one.: the problem is that sk->f2 is modifed in PKE_Decrypt, which is quite strange. randombytes should also be deleted to use randombytes in pqm4.

![Parameter for Dawn](image.png)

3. NTT, basemul, INTT.