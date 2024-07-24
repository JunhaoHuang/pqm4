.syntax unified
#include "macros.i"

// void raccoon_ntt(int32_t p[N], int32_t zeta[N], int32_t qqinv[2])
.global raccoon_ntt
.type raccoon_ntt,%function
.align 2
raccoon_ntt:
  //bind aliases
  ptr_p     .req R0
  ptr_zeta  .req R1
  zeta      .req R1
  q         .req R2
  qinv      .req R3
  pol4      .req R4
  pol0      .req R5
  pol1      .req R6
  pol2      .req R7
  pol3      .req R8
  temp_h    .req R9
  temp_l    .req R10
  pol5      .req R11
  pol6      .req R12
  pol7      .req R14

  //preserve registers
  push {R4-R11, R14}

  ldrd q, qinv, [q] // get q and qinv from an array.

  // state 1-3
  .equ distance, 1024
  .equ strincr, 4

  ldr ptr_zeta, =#zetas_ntt
  vldm ptr_zeta!, {s2-s8}
  vmov s0, ptr_zeta

  add.w temp_l, ptr_p, #64*strincr // 64 iterations
  vmov s9, temp_l
  1:
    .rept 2
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #1*distance/4]
    ldr.w pol2, [ptr_p, #2*distance/4]
    ldr.w pol3, [ptr_p, #3*distance/4]
    ldr.w pol4, [ptr_p, #4*distance/4]
    ldr.w pol5, [ptr_p, #5*distance/4]
    ldr.w pol6, [ptr_p, #6*distance/4]
    ldr.w pol7, [ptr_p, #7*distance/4]

    _3_layer_CT_32 pol0, pol1, pol2, pol3, pol4, pol5, pol6, pol7, s2, s3, s4, s5, s6, s7, s8, zeta, qinv, q, temp_h, temp_l

    str.w pol1, [ptr_p, #1*distance/4]
    str.w pol2, [ptr_p, #2*distance/4]
    str.w pol3, [ptr_p, #3*distance/4]
    str.w pol4, [ptr_p, #4*distance/4]
    str.w pol5, [ptr_p, #5*distance/4]
    str.w pol6, [ptr_p, #6*distance/4]
    str.w pol7, [ptr_p, #7*distance/4]
    str.w pol0, [ptr_p], #strincr
    .endr
    vmov temp_l, s9
    cmp.w ptr_p, temp_l
    bne 1b
  
  sub ptr_p, #64*4

  // stage 4-6
  .equ distance, 128
  add.w temp_l, ptr_p, #8*224+8*8*4 // 8 iterations
  vmov s9, temp_l
  2:
    add.w temp_l, ptr_p, #8*strincr // 8 iterations
    vmov s10, temp_l
    vmov ptr_zeta, s0
    vldm ptr_zeta!, {s2-s8}
    vmov s0, ptr_zeta
    3:
      .rept 2
      ldr.w pol0, [ptr_p]
      ldr.w pol1, [ptr_p, #1*distance/4]
      ldr.w pol2, [ptr_p, #2*distance/4]
      ldr.w pol3, [ptr_p, #3*distance/4]
      ldr.w pol4, [ptr_p, #4*distance/4]
      ldr.w pol5, [ptr_p, #5*distance/4]
      ldr.w pol6, [ptr_p, #6*distance/4]
      ldr.w pol7, [ptr_p, #7*distance/4]

      _3_layer_CT_32 pol0, pol1, pol2, pol3, pol4, pol5, pol6, pol7, s2, s3, s4, s5, s6, s7, s8, zeta, qinv, q, temp_h, temp_l
      
      str.w pol1, [ptr_p, #1*distance/4]
      str.w pol2, [ptr_p, #2*distance/4]
      str.w pol3, [ptr_p, #3*distance/4]
      str.w pol4, [ptr_p, #4*distance/4]
      str.w pol5, [ptr_p, #5*distance/4]
      str.w pol6, [ptr_p, #6*distance/4]
      str.w pol7, [ptr_p, #7*distance/4]
      str.w pol0, [ptr_p], #4
      .endr
      vmov temp_l, s10
      cmp.w ptr_p, temp_l
      bne 3b

    add.w ptr_p, #224
    vmov temp_l, s9
    cmp.w ptr_p, temp_l
  bne 2b
  
  .equ distance, 16
  vmov s9, ptr_p
  sub ptr_p, #8*224+8*8*4
  // stage 7-9
  // add.w temp_l, ptr_p, #2048 // 64 iterations
  4:
    vmov ptr_zeta, s0
    vldm ptr_zeta!, {s2-s8}
    vmov s0, ptr_zeta
    ldr.w pol0, [ptr_p]
    ldr.w pol1, [ptr_p, #1*distance/4]
    ldr.w pol2, [ptr_p, #2*distance/4]
    ldr.w pol3, [ptr_p, #3*distance/4]
    ldr.w pol4, [ptr_p, #4*distance/4]
    ldr.w pol5, [ptr_p, #5*distance/4]
    ldr.w pol6, [ptr_p, #6*distance/4]
    ldr.w pol7, [ptr_p, #7*distance/4]

    _3_layer_CT_32 pol0, pol1, pol2, pol3, pol4, pol5, pol6, pol7, s2, s3, s4, s5, s6, s7, s8, zeta, qinv, q, temp_h, temp_l
    
    str.w pol1, [ptr_p, #1*distance/4]
    str.w pol2, [ptr_p, #2*distance/4]
    str.w pol3, [ptr_p, #3*distance/4]
    str.w pol4, [ptr_p, #4*distance/4]
    str.w pol5, [ptr_p, #5*distance/4]
    str.w pol6, [ptr_p, #6*distance/4]
    str.w pol7, [ptr_p, #7*distance/4]
    str.w pol0, [ptr_p], #32

    vmov temp_l, s9
    cmp.w temp_l, ptr_p
  bne.w 4b

  //restore registers
  pop {R4-R11, PC}  