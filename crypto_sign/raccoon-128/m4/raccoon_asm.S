.syntax unified
#include "macros.i"

// void polyr2_split(int64_t p[N]) in-place
.global polyr2_split_asm
.type polyr2_split_asm,%function
.align 2
polyr2_split_asm:
  //bind aliases
  ptr_p     .req R0
  qinv2     .req R1
  q         .req R2
  qinv      .req R3
  pol4      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  temp_h    .req R9
  ctrn      .req R10
  pol5      .req R11
  pol6      .req R12
  temp_l    .req R14

  //preserve registers
  push {R4-R11, R14}

  //polyr2_split
  ldr.w qinv, ntt_asm_qinv1
  ldr.w q, ntt_asm_q1
  ldr.w qinv2, ntt_asm_qinv2
  ldr.w pol6, ntt_asm_q2
  
  add.w ctrn, ptr_p, #4096-4*4
  1:
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #4*1]
    ldr.w pol2, [ptr_p, #4*2]
    ldr.w pol3, [ptr_p, #4*3]
    ldr.w pol4, [ptr_p, #4*4]
    ldr.w pol5, [ptr_p, #4*5]

    montgomery_redc_v2 pol0, pol1, qinv, q, qinv2, pol6, temp_l, temp_h
    montgomery_redc_v2 pol2, pol3, qinv, q, qinv2, pol6, temp_l, temp_h
    montgomery_redc_v2 pol4, pol5, qinv, q, qinv2, pol6, temp_l, temp_h

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol0, [ptr_p], #4*6
  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w pol0, [ptr_p]
  ldr.w pol1, [ptr_p, #4*1]
  ldr.w pol2, [ptr_p, #4*2]
  ldr.w pol3, [ptr_p, #4*3]

  montgomery_redc_v2 pol0, pol1, qinv, q, qinv2, pol6, temp_l, temp_h
  montgomery_redc_v2 pol2, pol3, qinv, q, qinv2, pol6, temp_l, temp_h

  str.w pol1, [ptr_p, #4*1]
  str.w pol2, [ptr_p, #4*2]
  str.w pol3, [ptr_p, #4*3]
  str.w pol0, [ptr_p], #4*4

  pop {R4-R11, PC}

.ltorg

.global polyr2_split_neg_asm
.type polyr2_split_neg_asm,%function
.align 2
polyr2_split_neg_asm:
  //bind aliases
  ptr_p     .req R0
  qinv2     .req R1
  q         .req R2
  qinv      .req R3
  pol4      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  temp_h    .req R9
  ctrn      .req R10
  pol5      .req R11
  pol6      .req R12
  temp_l    .req R14

  //preserve registers
  push {R4-R11, R14}

  //polyr2_split
  ldr.w qinv, ntt_asm_qinv1
  ldr.w q, ntt_asm_q1
  ldr.w qinv2, ntt_asm_qinv2
  ldr.w pol6, ntt_asm_q2
  
  add.w ctrn, ptr_p, #4096-4*4
  1:
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #4*1]
    ldr.w pol2, [ptr_p, #4*2]
    ldr.w pol3, [ptr_p, #4*3]
    ldr.w pol4, [ptr_p, #4*4]
    ldr.w pol5, [ptr_p, #4*5]

    montgomery_redc_v3 pol0, pol1, qinv, q, qinv2, pol6, temp_l, temp_h
    montgomery_redc_v3 pol2, pol3, qinv, q, qinv2, pol6, temp_l, temp_h
    montgomery_redc_v3 pol4, pol5, qinv, q, qinv2, pol6, temp_l, temp_h

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol0, [ptr_p], #4*6
  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w pol0, [ptr_p]
  ldr.w pol1, [ptr_p, #4*1]
  ldr.w pol2, [ptr_p, #4*2]
  ldr.w pol3, [ptr_p, #4*3]

  montgomery_redc_v3 pol0, pol1, qinv, q, qinv2, pol6, temp_l, temp_h
  montgomery_redc_v3 pol2, pol3, qinv, q, qinv2, pol6, temp_l, temp_h

  str.w pol1, [ptr_p, #4*1]
  str.w pol2, [ptr_p, #4*2]
  str.w pol3, [ptr_p, #4*3]
  str.w pol0, [ptr_p], #4*4

  pop {R4-R11, PC}

  // unbind aliases
  .unreq ptr_p
  .unreq qinv2
  .unreq q
  .unreq qinv
  .unreq pol4
  .unreq pol0
  .unreq pol1
  .unreq pol2
  .unreq pol3
  .unreq temp_h
  .unreq ctrn
  .unreq pol5
  .unreq pol6
  .unreq temp_l

.ltorg

// void polyr2_join(int64_t p[N], int32_t s1, int32_t s2)
.global polyr2_join_asm
.type polyr2_join_asm,%function
.align 2
polyr2_join_asm:
  //bind aliases
  ptr_p     .req R0
  scal1     .req R1
  scal2     .req R2
  qinv      .req R3
  q         .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  qinv2     .req R9
  ctrn      .req R10
  qq2       .req R11
  temp_l    .req R12
  temp_h    .req R14

  push {R4-R11, R14}

  ldr.w qinv, ntt_asm_minus_qinv1
  ldr.w qinv2, ntt_asm_minus_qinv2

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #4*1]
    ldr.w pol2, [ptr_p, #4*2]
    ldr.w pol3, [ptr_p, #4*3]
    ldr.w q, ntt_asm_q1
    ldr.w qq2, ntt_asm_q2

    montgomery_mul_32 pol0, scal1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol2, scal1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol1, scal2, qinv2, qq2, temp_l, temp_h
    montgomery_mul_32 pol3, scal2, qinv2, qq2, temp_l, temp_h

    mont32_cadd_asm pol0, q, temp_l
    mont32_cadd_asm pol2, q, temp_l
    mont32_cadd_asm pol1, qq2, temp_l
    mont32_cadd_asm pol3, qq2, temp_l

    smull pol0, temp_l, pol0, qq2
    smlal pol0, temp_l, pol1, q
    smull pol2, temp_h, pol2, qq2
    smlal pol2, temp_h, pol3, q

    smull q, qq2, q, qq2
    mont64_csub_asm pol0, temp_l, q, qq2, pol1, pol3
    mont64_csub_asm pol2, temp_h, q, qq2, pol1, pol3

    str.w temp_l, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w temp_h, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4
  cmp.w ptr_p, ctrn
  bne 1b

  pop {R4-R11, PC}
  .unreq ptr_p 
  .unreq scal1 
  .unreq scal2 
  .unreq qinv  
  .unreq q     
  .unreq pol0  
  .unreq pol1  
  .unreq pol2  
  .unreq pol3  
  .unreq qinv2 
  .unreq ctrn  
  .unreq qq2   
  .unreq temp_l
  .unreq temp_h

.ltorg

// void polyr2_add(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr2_add_asm
.type polyr2_add_asm,%function
.align 2
polyr2_add_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14
  push {R4-R11, R14}

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    add pol0, pol4
    add pol1, pol5
    add pol2, pol6
    add pol3, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr2_ntt_addq(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr2_ntt_addq_asm
.type polyr2_ntt_addq_asm,%function
.align 2
polyr2_ntt_addq_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  ldr.w q, ntt_asm_q1
  ldr.w qq2, ntt_asm_q2

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    add pol0, pol4
    add pol1, pol5
    add pol2, pol6
    add pol3, pol7

    mont32_csub_asm pol0, q, pol4
    mont32_csub_asm pol2, q, pol4
    mont32_csub_asm pol1, qq2, pol4
    mont32_csub_asm pol3, qq2, pol4

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

.ltorg

// void polyr2_reduce_q2(int64_t *r, const int64_t *a) only perform modular reduction mod q_2
.global polyr2_reduce_q2_asm
.type polyr2_reduce_q2_asm,%function
.align 2
polyr2_reduce_q2_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  tmp       .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  ldr.w qq2, ntt_asm_q2

  add.w ptr_p, #4
  add.w ptr_a, #4
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol2, [ptr_a, #8*2]
    ldr.w pol3, [ptr_a, #8*3]
    ldr.w pol4, [ptr_a, #8*4]
    ldr.w pol5, [ptr_a, #8*5]
    ldr.w pol6, [ptr_a, #8*6]
    ldr.w pol7, [ptr_a, #8*7]
    ldr.w pol0, [ptr_a], #8*8

    reduce_q2_asm pol0, qq2 tmp
    reduce_q2_asm pol1, qq2 tmp
    reduce_q2_asm pol2, qq2 tmp
    reduce_q2_asm pol3, qq2 tmp
    reduce_q2_asm pol4, qq2, tmp
    reduce_q2_asm pol5, qq2, tmp
    reduce_q2_asm pol6, qq2, tmp
    reduce_q2_asm pol7, qq2, tmp

    str.w pol1, [ptr_p, #8*1]
    str.w pol2, [ptr_p, #8*2]
    str.w pol3, [ptr_p, #8*3]
    str.w pol4, [ptr_p, #8*4]
    str.w pol5, [ptr_p, #8*5]
    str.w pol6, [ptr_p, #8*6]
    str.w pol7, [ptr_p, #8*7]
    str.w pol0, [ptr_p], #8*8

  cmp.w ptr_p, ctrn
  bne 1b

  pop {R4-R11, PC}

.ltorg


// void polyr2_reduce(int64_t *r, const int64_t *a)
.global polyr2_reduce_asm
.type polyr2_reduce_asm,%function
.align 2
polyr2_reduce_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  tmp       .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  ldr.w q, ntt_asm_q1
  ldr.w qq2, ntt_asm_q2

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    reduce_q1_asm pol0, q, tmp
    reduce_q1_asm pol2, q, tmp
    reduce_q1_asm pol4, q, tmp
    reduce_q1_asm pol6, q, tmp
    reduce_q2_asm pol1, qq2, tmp
    reduce_q2_asm pol3, qq2, tmp
    reduce_q2_asm pol5, qq2, tmp
    reduce_q2_asm pol7, qq2, tmp

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8

  cmp.w ptr_p, ctrn
  bne 1b

  pop {R4-R11, PC}
  .unreq tmp

.ltorg

// void polyr2_sub(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr2_sub_asm
.type polyr2_sub_asm,%function
.align 2
polyr2_sub_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14
  push {R4-R11, R14}

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    sub pol0, pol4
    sub pol1, pol5
    sub pol2, pol6
    sub pol3, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}
  .unreq ptr_b


// void polyr_reduce(int64_t *r, const int64_t *a)
.global polyr_reduce_asm
.type polyr_reduce_asm,%function
.align 2
polyr_reduce_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  tmp       .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  ldr.w q, ntt_asm_q1
  ldr.w qq2, ntt_asm_q2
  smull q, qq2, q, qq2

  add.w ctrn, ptr_p, #4096-4*4
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol0, [ptr_a], #4*6

    reduce64_asm pol0, pol1, q, qq2, tmp, pol6, pol7
    reduce64_asm pol2, pol3, q, qq2, tmp, pol6, pol7
    reduce64_asm pol4, pol5, q, qq2, tmp, pol6, pol7
    mont64_cadd_asm pol0, pol1, q, qq2, tmp, pol6
    mont64_cadd_asm pol2, pol3, q, qq2, tmp, pol6
    mont64_cadd_asm pol4, pol5, q, qq2, tmp, pol6

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol0, [ptr_p], #4*6

  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w pol1, [ptr_a, #4*1]
  ldr.w pol2, [ptr_a, #4*2]
  ldr.w pol3, [ptr_a, #4*3]
  ldr.w pol0, [ptr_a], #4*4

  reduce64_asm pol0, pol1, q, qq2, tmp, pol6, pol7
  reduce64_asm pol2, pol3, q, qq2, tmp, pol6, pol7
  mont64_cadd_asm pol0, pol1, q, qq2, tmp, pol6
  mont64_cadd_asm pol2, pol3, q, qq2, tmp, pol6

  str.w pol1, [ptr_p, #4*1]
  str.w pol2, [ptr_p, #4*2]
  str.w pol3, [ptr_p, #4*3]
  str.w pol0, [ptr_p], #4*4

  pop {R4-R11, PC}

.ltorg


// void polyr_sub(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_sub_asm
.type polyr_sub_asm,%function
.align 2
polyr_sub_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_sub_asm pol0, pol1, pol4, pol5
    mont64_sub_asm pol2, pol3, pol6, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}


// void polyr_add(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_add_asm
.type polyr_add_asm,%function
.align 2
polyr_add_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qq2       .req R14

  push {R4-R11, R14}

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_add_asm pol0, pol1, pol4, pol5
    mont64_add_asm pol2, pol3, pol6, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

  .unreq ptr_p
  .unreq ptr_a
  .unreq ptr_b
  .unreq pol4 
  .unreq pol5 
  .unreq pol0 
  .unreq pol1 
  .unreq pol2 
  .unreq pol3 
  .unreq pol6 
  .unreq ctrn 
  .unreq pol7 
  .unreq q    
  .unreq qq2  

.ltorg

  // void polyr_ntt_smul(int64_t *r, const int64_t *a, int32_t c1, int32_t c2)
.global polyr_ntt_smul_asm
.type polyr_ntt_smul_asm,%function
.align 2
polyr_ntt_smul_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  con1      .req R2
  con2      .req R3
  temp_l    .req R3
  pol4      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol5      .req R9
  ctrn      .req R10
  temp_h    .req R11
  q         .req R12
  qinv      .req R14

  push {R4-R11, R14}
  ldr.w q, ntt_asm_q1
  ldr.w qinv, ntt_asm_minus_qinv1
  vmov s0, con2

  add.w ctrn, ptr_p, #4096-8*2
  1:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol2, [ptr_a, #8*2]
    ldr.w pol3, [ptr_a, #8*3]
    ldr.w pol4, [ptr_a, #8*4]
    ldr.w pol5, [ptr_a, #8*5]
    ldr.w pol0, [ptr_a], #8*6
    
    montgomery_mul_32 pol0, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol1, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol2, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol3, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol4, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol5, con1, qinv, q, temp_l, temp_h

    str.w pol1, [ptr_p, #8*1]
    str.w pol2, [ptr_p, #8*2]
    str.w pol3, [ptr_p, #8*3]
    str.w pol4, [ptr_p, #8*4]
    str.w pol5, [ptr_p, #8*5]
    str.w pol0, [ptr_p], #8*6

  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w pol1, [ptr_a, #8*1]
  ldr.w pol0, [ptr_a], #8*2

  montgomery_mul_32 pol0, con1, qinv, q, temp_l, temp_h
  montgomery_mul_32 pol1, con1, qinv, q, temp_l, temp_h
  
  ldr.w q, ntt_asm_q2
  ldr.w qinv, ntt_asm_minus_qinv2
  str.w pol1, [ptr_p, #8*1]
  str.w pol0, [ptr_p], #8*2

  sub ptr_a, #4096-4
  sub ptr_p, #4096-4
  add ctrn, #4
  vmov con1, s0
  2:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol2, [ptr_a, #8*2]
    ldr.w pol3, [ptr_a, #8*3]
    ldr.w pol4, [ptr_a, #8*4]
    ldr.w pol5, [ptr_a, #8*5]
    ldr.w pol0, [ptr_a], #8*6
    
    montgomery_mul_32 pol0, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol1, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol2, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol3, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol4, con1, qinv, q, temp_l, temp_h
    montgomery_mul_32 pol5, con1, qinv, q, temp_l, temp_h

    str.w pol1, [ptr_p, #8*1]
    str.w pol2, [ptr_p, #8*2]
    str.w pol3, [ptr_p, #8*3]
    str.w pol4, [ptr_p, #8*4]
    str.w pol5, [ptr_p, #8*5]
    str.w pol0, [ptr_p], #8*6
  cmp.w ptr_p, ctrn
  bne 2b

  ldr.w pol1, [ptr_a, #8*1]
  ldr.w pol0, [ptr_a], #8*2

  montgomery_mul_32 pol0, con1, qinv, q, temp_l, temp_h
  montgomery_mul_32 pol1, con1, qinv, q, temp_l, temp_h
  
  str.w pol1, [ptr_p, #8*1]
  str.w pol0, [ptr_p], #8*2
  pop {R4-R11, PC}

  .unreq con1  
  .unreq con2  
  .unreq temp_l
  .unreq pol4  
  .unreq pol0  
  .unreq pol1  
  .unreq pol2  
  .unreq pol3  
  .unreq pol5  
  .unreq ctrn  
  .unreq temp_h
  .unreq q     
  .unreq qinv  

// pb will be modified
.macro montgomery_multiplication pa, pb, qinv, q, tmp
    smull \tmp, \pa, \pa, \pb
    mul \pb, \tmp, \qinv
    smlal \tmp, \pa, \pb, \q
.endm

// void polyr_ntt_cmul(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_ntt_cmul_asm
.type polyr_ntt_cmul_asm,%function
.align 2
polyr_ntt_cmul_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  q         .req R12
  qinv      .req R14

  push {R4-R11, R14}

  ldr.w q, ntt_asm_q1
  ldr.w qinv, ntt_asm_minus_qinv1
  add.w ctrn, ptr_p, #4096
  vmov s0, ctrn
  1:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol2, [ptr_a, #8*2]
    ldr.w pol3, [ptr_a, #8*3]
    ldr.w pol0, [ptr_a], #8*4
    ldr.w pol5, [ptr_b, #8*1]
    ldr.w pol6, [ptr_b, #8*2]
    ldr.w pol7, [ptr_b, #8*3]
    ldr.w pol4, [ptr_b], #8*4
    
    montgomery_multiplication pol0, pol4, qinv, q, ctrn
    montgomery_multiplication pol1, pol5, qinv, q, ctrn
    montgomery_multiplication pol2, pol6, qinv, q, ctrn
    montgomery_multiplication pol3, pol7, qinv, q, ctrn

    str.w pol1, [ptr_p, #8*1]
    str.w pol2, [ptr_p, #8*2]
    str.w pol3, [ptr_p, #8*3]
    str.w pol0, [ptr_p], #8*4
  vmov ctrn, s0
  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w q, ntt_asm_q2
  ldr.w qinv, ntt_asm_minus_qinv2
  sub ptr_a, #4096-4
  sub ptr_b, #4096-4
  sub ptr_p, #4096-4
  add ctrn, #4
  vmov s0, ctrn
  2:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol2, [ptr_a, #8*2]
    ldr.w pol3, [ptr_a, #8*3]
    ldr.w pol0, [ptr_a], #8*4
    ldr.w pol5, [ptr_b, #8*1]
    ldr.w pol6, [ptr_b, #8*2]
    ldr.w pol7, [ptr_b, #8*3]
    ldr.w pol4, [ptr_b], #8*4
    
    montgomery_multiplication pol0, pol4, qinv, q, ctrn
    montgomery_multiplication pol1, pol5, qinv, q, ctrn
    montgomery_multiplication pol2, pol6, qinv, q, ctrn
    montgomery_multiplication pol3, pol7, qinv, q, ctrn

    str.w pol1, [ptr_p, #8*1]
    str.w pol2, [ptr_p, #8*2]
    str.w pol3, [ptr_p, #8*3]
    str.w pol0, [ptr_p], #8*4
  vmov ctrn, s0
  cmp.w ptr_p, ctrn
  bne 2b

  pop {R4-R11, PC}

  .unreq pol4
  .unreq pol5
  .unreq pol0
  .unreq pol1
  .unreq pol2
  .unreq pol3
  .unreq pol6
  .unreq ctrn
  .unreq pol7
  .unreq q   
  .unreq qinv

// void polyr_ntt_mula(int64_t *r, const int64_t *a, const int64_t *b, const int64_t *c)
.global polyr_ntt_mula_asm
.type polyr_ntt_mula_asm,%function
.align 2
polyr_ntt_mula_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  ptr_c     .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  temp      .req R9
  ctrn      .req R10
  pol4      .req R11
  q         .req R12
  qinv      .req R14

  push {R4-R11, R14}

  ldr.w q, ntt_asm_q1
  ldr.w qinv, ntt_asm_minus_qinv1
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol0, [ptr_a], #8*2
    ldr.w pol3, [ptr_b, #8*1]
    ldr.w pol2, [ptr_b], #8*2
    ldr.w pol5, [ptr_c, #8*1]
    ldr.w pol4, [ptr_c], #8*2
    
    montgomery_multiplication pol0, pol2, qinv, q, temp
    montgomery_multiplication pol1, pol3, qinv, q, temp
    add pol0, pol4
    add pol1, pol5

    str.w pol1, [ptr_p, #8*1]
    str.w pol0, [ptr_p], #8*2
  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w q, ntt_asm_q2
  ldr.w qinv, ntt_asm_minus_qinv2
  sub ptr_a, #4096-4
  sub ptr_b, #4096-4
  sub ptr_c, #4096-4
  sub ptr_p, #4096-4
  add ctrn, #4
  2:
    ldr.w pol1, [ptr_a, #8*1]
    ldr.w pol0, [ptr_a], #8*2
    ldr.w pol3, [ptr_b, #8*1]
    ldr.w pol2, [ptr_b], #8*2
    ldr.w pol5, [ptr_c, #8*1]
    ldr.w pol4, [ptr_c], #8*2
    
    montgomery_multiplication pol0, pol2, qinv, q, temp
    montgomery_multiplication pol1, pol3, qinv, q, temp
    add pol0, pol4
    add pol1, pol5

    str.w pol1, [ptr_p, #8*1]
    str.w pol0, [ptr_p], #8*2
  cmp.w ptr_p, ctrn
  bne 2b

  pop {R4-R11, PC}
.unreq ptr_p
.unreq ptr_a
.unreq ptr_b
.unreq ptr_c
.unreq pol5 
.unreq pol0 
.unreq pol1 
.unreq pol2 
.unreq pol3 
.unreq temp 
.unreq ctrn 
.unreq pol4 
.unreq q    
.unreq qinv 

.ltorg

// void polyr_subq(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_subq_asm
.type polyr_subq_asm,%function
.align 2
polyr_subq_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  qLo       .req R12
  qHi       .req R14

  push {R4-R11, R14}

  ldr.w qLo, ntt_asm_q1
  ldr.w qHi, ntt_asm_q2
  smull qLo, qHi, qLo, qHi

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_sub_asm pol0, pol1, pol4, pol5
    mont64_sub_asm pol2, pol3, pol6, pol7

    mont64_cadd_asm pol0, pol1, qLo, qHi, pol4, pol5
    mont64_cadd_asm pol2, pol3, qLo, qHi, pol4, pol5

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

.ltorg

// void polyr_addq(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_addq_asm
.type polyr_addq_asm,%function
.align 2
polyr_addq_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  pol4      .req R3
  pol5      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  qLo       .req R12
  qHi       .req R14

  push {R4-R11, R14}

  ldr.w qLo, ntt_asm_q1
  ldr.w qHi, ntt_asm_q2
  smull qLo, qHi, qLo, qHi

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_add_asm pol0, pol1, pol4, pol5
    mont64_add_asm pol2, pol3, pol6, pol7
    mont64_csub_asm pol0, pol1, qLo, qHi, pol4, pol5
    mont64_csub_asm pol2, pol3, qLo, qHi, pol4, pol5

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}
  .unreq ptr_p
  .unreq ptr_a
  .unreq ptr_b
  .unreq pol4 
  .unreq pol5 
  .unreq pol0 
  .unreq pol1 
  .unreq pol2 
  .unreq pol3 
  .unreq pol6 
  .unreq ctrn 
  .unreq pol7 
  .unreq qLo    
  .unreq qHi
  
// void polyr_addm(int64_t *r, const int64_t *a, const int64_t *b, int64_t* m)
.global polyr_addm_asm
.type polyr_addm_asm,%function
.align 2
polyr_addm_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  ptr_q     .req R3
  qLo       .req R3
  qHi       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  
  push {R4-R11, R14}

  ldrd.w qLo, qHi, [ptr_q]

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_add_asm pol0, pol1, pol4, pol5
    mont64_add_asm pol2, pol3, pol6, pol7
    mont64_csub_asm pol0, pol1, qLo, qHi, pol4, pol5
    mont64_csub_asm pol2, pol3, qLo, qHi, pol4, pol5

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr_subm(int64_t *r, const int64_t *a, const int64_t *b, int64_t* m)
.global polyr_subm_asm
.type polyr_subm_asm,%function
.align 2
polyr_subm_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  ptr_b     .req R2
  ptr_q     .req R3
  qLo       .req R3
  qHi       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}
  ldrd.w qLo, qHi, [ptr_q]

  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol0, [ptr_a], #4*4
    ldr.w pol5, [ptr_b, #4*1]
    ldr.w pol6, [ptr_b, #4*2]
    ldr.w pol7, [ptr_b, #4*3]
    ldr.w pol4, [ptr_b], #4*4

    mont64_sub_asm pol0, pol1, pol4, pol5
    mont64_sub_asm pol2, pol3, pol6, pol7
    mont64_cadd_asm pol0, pol1, qLo, qHi, pol4, pol5
    mont64_cadd_asm pol2, pol3, qLo, qHi, pol4, pol5

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol0, [ptr_p], #4*4

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

  .unreq ptr_b
  .unreq qLo    
  .unreq qHi
  .unreq tmp

// void polyr_shlm42(int64_t *r, const int64_t *a) tailored for 42 in raccoon
.global polyr_shlm42_asm
.type polyr_shlm42_asm,%function
.align 2
polyr_shlm42_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  movs tmp, #0 // all zero
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    lsls pol1, pol0, #10
    lsls pol3, pol2, #10
    lsls pol5, pol4, #10
    lsls pol7, pol6, #10

    str.w pol1, [ptr_p, #4*1]
    str.w tmp, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w tmp, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w tmp, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w tmp, [ptr_p], #4*8

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}
  .unreq ptr_a
  .unreq qLo
  .unreq qHi

// void polyr_shrm42(int64_t *r, int32_t q) tailored for 42 in raccoon
.global polyr_shrm42_asm
.type polyr_shrm42_asm,%function
.align 2
polyr_shrm42_asm:
  //bind aliases
  ptr_p     .req R0
  q         .req R1
  RC        .req R2
  tmp2      .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}
  
  mov tmp2, #0
  mov RC, #512 //1<<(42-32-1)
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #4*1]
    ldr.w pol2, [ptr_p, #4*2]
    ldr.w pol3, [ptr_p, #4*3]
    ldr.w pol4, [ptr_p, #4*4]
    ldr.w pol5, [ptr_p, #4*5]
    ldr.w pol6, [ptr_p, #4*6]
    ldr.w pol7, [ptr_p, #4*7]
    

    add pol1, RC
    add pol3, RC
    add pol5, RC
    add pol7, RC
    rsb pol0, q, pol1, asr #10
    rsb pol2, q, pol3, asr #10
    rsb pol4, q, pol5, asr #10
    rsb pol6, q, pol7, asr #10
    and tmp, q, pol0, asr #31
    add pol0, tmp
    and tmp, q, pol2, asr #31
    add pol2, tmp
    and tmp, q, pol4, asr #31
    add pol4, tmp
    and tmp, q, pol6, asr #31
    add pol6, tmp

    // pol1, pol3, pol5, pol7 are all 0 now
    str.w tmp2, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w tmp2, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w tmp2, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w tmp2, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr_shrm44(int64_t *r, int32_t q) tailored for 44 in raccoon
.global polyr_shrm44_asm
.type polyr_shrm44_asm,%function
.align 2
polyr_shrm44_asm:
  //bind aliases
  ptr_p     .req R0
  q         .req R1
  RC        .req R2
  tmp2      .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}
  
  mov RC, #2048 //1<<(44-32-1)
  mov tmp2, #0
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #4*1]
    ldr.w pol2, [ptr_p, #4*2]
    ldr.w pol3, [ptr_p, #4*3]
    ldr.w pol4, [ptr_p, #4*4]
    ldr.w pol5, [ptr_p, #4*5]
    ldr.w pol6, [ptr_p, #4*6]
    ldr.w pol7, [ptr_p, #4*7]

    add pol1, RC
    add pol3, RC
    add pol5, RC
    add pol7, RC
    rsb pol0, q, pol1, asr #12
    rsb pol2, q, pol3, asr #12
    rsb pol4, q, pol5, asr #12
    rsb pol6, q, pol7, asr #12
    and tmp, q, pol0, asr #31
    add pol0, tmp
    and tmp, q, pol2, asr #31
    add pol2, tmp
    and tmp, q, pol4, asr #31
    add pol4, tmp
    and tmp, q, pol6, asr #31
    add pol6, tmp

    str.w tmp2, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w tmp2, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w tmp2, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w tmp2, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8

  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

  // void polyr_negm(int64_t *r, int64_t *a, int64_t m)
.global polyr_negm_asm
.type polyr_negm_asm,%function
.align 2
polyr_negm_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  // ldrd qLo, qHi, [qLo]
  add.w ctrn, ptr_p, #4096
  vmov s0, ctrn
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    mont64_neg_asm pol0, pol1
    mont64_neg_asm pol2, pol3
    mont64_neg_asm pol4, pol5
    mont64_neg_asm pol6, pol7
    mont64_cadd_asm pol0, pol1, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol2, pol3, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol4, pol5, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol6, pol7, qLo, qHi, ctrn, tmp

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8
  vmov ctrn, s0
  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr_neg(int64_t *r, int64_t *a)
.global polyr_neg_asm
.type polyr_neg_asm,%function
.align 2
polyr_neg_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  // ldrd qLo, qHi, [qLo]
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    mont64_neg_asm pol0, pol1
    mont64_neg_asm pol2, pol3
    mont64_neg_asm pol4, pol5
    mont64_neg_asm pol6, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8
  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr2_neg(int64_t *r, int64_t *a)
.global polyr2_neg_asm
.type polyr2_neg_asm,%function
.align 2
polyr2_neg_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  // ldrd qLo, qHi, [qLo]
  add.w ctrn, ptr_p, #4096
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    neg pol0, pol0
    neg pol1, pol1
    neg pol2, pol2
    neg pol3, pol3
    neg pol4, pol4
    neg pol5, pol5
    neg pol6, pol6
    neg pol7, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8
  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}

// void polyr_nonneg(int64_t *r, int64_t *a, int64_t m)
.global polyr_nonneg_asm
.type polyr_nonneg_asm,%function
.align 2
polyr_nonneg_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  add.w ctrn, ptr_p, #4096
  vmov s0, ctrn
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol6, [ptr_a, #4*6]
    ldr.w pol7, [ptr_a, #4*7]
    ldr.w pol0, [ptr_a], #4*8

    mont64_cadd_asm pol0, pol1, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol2, pol3, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol4, pol5, qLo, qHi, ctrn, tmp
    mont64_cadd_asm pol6, pol7, qLo, qHi, ctrn, tmp

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol6, [ptr_p, #4*6]
    str.w pol7, [ptr_p, #4*7]
    str.w pol0, [ptr_p], #4*8
  vmov ctrn, s0
  cmp.w ptr_p, ctrn
  bne 1b
  pop {R4-R11, PC}
  
// 32-bit: m
// void polyr_center(int64_t *r, int64_t *a, int64_t m)
.global polyr_center_asm
.type polyr_center_asm,%function
.align 2
polyr_center_asm:
  //bind aliases
  ptr_p     .req R0
  ptr_a     .req R1
  qLo       .req R2
  qHi       .req R3
  tmp       .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  pol6      .req R9
  ctrn      .req R10
  pol7      .req R11
  pol4      .req R12
  pol5      .req R14

  push {R4-R11, R14}

  // m>>1 -> pol6, pol7
  lsrs pol6, qLo, #1
  orr pol6, pol6, qHi, lsl #31
  asrs pol7, qHi, #1

  add.w ctrn, ptr_p, #4096-4*4
  vmov s0, ctrn
  1:
    ldr.w pol1, [ptr_a, #4*1]
    ldr.w pol2, [ptr_a, #4*2]
    ldr.w pol3, [ptr_a, #4*3]
    ldr.w pol4, [ptr_a, #4*4]
    ldr.w pol5, [ptr_a, #4*5]
    ldr.w pol0, [ptr_a], #4*6

    mont64_add_asm pol0, pol1, pol6, pol7
    mont64_add_asm pol2, pol3, pol6, pol7
    mont64_add_asm pol4, pol5, pol6, pol7

    mont64_csub_asm pol0, pol1, qLo, qHi, ctrn, tmp
    mont64_csub_asm pol2, pol3, qLo, qHi, ctrn, tmp
    mont64_csub_asm pol4, pol5, qLo, qHi, ctrn, tmp

    mont64_sub_asm pol0, pol1, pol6, pol7
    mont64_sub_asm pol2, pol3, pol6, pol7
    mont64_sub_asm pol4, pol5, pol6, pol7

    str.w pol1, [ptr_p, #4*1]
    str.w pol2, [ptr_p, #4*2]
    str.w pol3, [ptr_p, #4*3]
    str.w pol4, [ptr_p, #4*4]
    str.w pol5, [ptr_p, #4*5]
    str.w pol0, [ptr_p], #4*6
  vmov ctrn, s0
  cmp.w ptr_p, ctrn
  bne 1b

  ldr.w pol1, [ptr_a, #4*1]
  ldr.w pol2, [ptr_a, #4*2]
  ldr.w pol3, [ptr_a, #4*3]
  ldr.w pol0, [ptr_a], #4*4

  mont64_add_asm pol0, pol1, pol6, pol7
  mont64_add_asm pol2, pol3, pol6, pol7

  mont64_csub_asm pol0, pol1, qLo, qHi, pol5, tmp
  mont64_csub_asm pol2, pol3, qLo, qHi, pol5, tmp

  mont64_sub_asm pol0, pol1, pol6, pol7
  mont64_sub_asm pol2, pol3, pol6, pol7

  str.w pol1, [ptr_p, #4*1]
  str.w pol2, [ptr_p, #4*2]
  str.w pol3, [ptr_p, #4*3]
  str.w pol0, [ptr_p], #4*4

  pop {R4-R11, PC}

  .unreq ptr_p
  .unreq ptr_a
  .unreq pol4 
  .unreq pol5 
  .unreq pol0 
  .unreq pol1 
  .unreq pol2 
  .unreq pol3 
  .unreq pol6 
  .unreq ctrn 
  .unreq pol7 
  .unreq qLo    
  .unreq qHi


