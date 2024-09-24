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


// void polyr_ntt_addq(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_ntt_addq_asm
.type polyr_ntt_addq_asm,%function
.align 2
polyr_ntt_addq_asm:
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

  // void polyr_ntt_subq(int64_t *r, const int64_t *a, const int64_t *b)
.global polyr_ntt_subq_asm
.type polyr_ntt_subq_asm,%function
.align 2
polyr_ntt_subq_asm:
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

    sub pol0, pol4
    sub pol1, pol5
    sub pol2, pol6
    sub pol3, pol7

    mont32_cadd_asm pol0, q, pol4
    mont32_cadd_asm pol2, q, pol4
    mont32_cadd_asm pol1, qq2, pol4
    mont32_cadd_asm pol3, qq2, pol4

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


.align 2
ntt_asm_qinv1:
.word 4278452225
.align 2
ntt_asm_q1:
.word 16515073
.align 2
ntt_asm_qinv2:
.word 4261675009
.align 2
ntt_asm_q2:
.word 33292289
.align 2
ntt_asm_minus_qinv1:
.word 16515071
.align 2
ntt_asm_minus_qinv2:
.word 33292287
