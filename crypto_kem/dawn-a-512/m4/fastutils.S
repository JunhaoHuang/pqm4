/**
 * Copyright (c) 2023 Junhao Huang (jhhuang_nuaa@126.com)
 *
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "macros.i"

.syntax unified
.cpu cortex-m4
.thumb

###################################
#### check poly inverse in Zq #####
#### r0: a ########################
###################################
.align 2
.global check_poly_inv_Zq_asm
.type check_poly_inv_Zq_asm, %function
check_poly_inv_Zq_asm:
    push {r4-r6, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    tmp     .req r5
    ones    .req r12
    loop    .req r6

    movw ones, #1
    movt ones, #1
    movw loop, #64

1:
    ldr.w poly0, [aptr], #4
    ldr.w poly1, [aptr], #4
    ldr.w poly2, [aptr], #4
    ldr.w poly3, [aptr], #4
    smuad tmp, poly0, ones
    smlad tmp, poly1, ones, tmp
    cbz tmp, 2f
    smuad tmp, poly2, ones
    smlad tmp, poly3, ones, tmp
    cbz tmp, 2f
    subs loop, #1
    bne 1b

    movs r0, #0
    pop {r4-r6, pc}

2:
    movs r0, #1
    pop {r4-r6, pc}

.unreq aptr
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq tmp
.unreq ones
.unreq loop


###################################
#### check poly inverse in Z2 #####
#### r0: a ########################
###################################
.align 2
.global check_poly_inv_Z2_asm
.type check_poly_inv_Z2_asm, %function
check_poly_inv_Z2_asm:
    push {r4-r6, lr}

    aptr    .req r0
    xacc0   .req r1
    xacc1   .req r2
    tmp0    .req r3
    tmp1    .req r4
    tmp2    .req r5
    tmp3    .req r6

    movs xacc0, #0
    movs xacc1, #0
    movw tmp3, #16

1:
    ldr.w tmp0, [aptr], #4
    ldr.w tmp1, [aptr], #4
    ldr.w tmp2, [aptr], #4
    ldr.w r12, [aptr], #4

    eor xacc0, xacc0, tmp0
    eor xacc1, xacc1, tmp1
    eor xacc0, xacc0, tmp2
    eor xacc1, xacc1, r12
    subs tmp3, #1
    bne 1b

    eor xacc0, xacc0, xacc1
    eor xacc0, xacc0, xacc0, ror #16
    uxth xacc0, xacc0
    cmp xacc0, #0
    ite eq
    moveq r0, #1
    movne r0, #0
    pop {r4-r6, pc}

.unreq aptr
.unreq xacc0
.unreq xacc1
.unreq tmp0
.unreq tmp1
.unreq tmp2
.unreq tmp3


###################################
#### polynomial rounding ##########
#### r0: a ########################
###################################
.align 2
.global poly_round_asm
.type poly_round_asm, %function
poly_round_asm:
    push {r4-r8, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    tmp     .req r5
    tmp2    .req r6
    bias    .req r7
    loop    .req r8
    scale   .req r12

    // 9362 = floor(2^16 / 7), used as the fixed-point reciprocal of 7.
    movw scale, #9362
    movw bias, #3
    movt bias, #3
    movw loop, #64

1:
    ldr.w poly0, [aptr, #0]
    ldr.w poly1, [aptr, #4]
    ldr.w poly2, [aptr, #8]
    ldr.w poly3, [aptr, #12]

    sadd16 poly0, poly0, bias
    smulwb tmp, scale, poly0
    smulwt tmp2, scale, poly0
    pkhbt poly0, tmp, tmp2, lsl #16

    sadd16 poly1, poly1, bias
    smulwb tmp, scale, poly1
    smulwt tmp2, scale, poly1
    pkhbt poly1, tmp, tmp2, lsl #16

    sadd16 poly2, poly2, bias
    smulwb tmp, scale, poly2
    smulwt tmp2, scale, poly2
    pkhbt poly2, tmp, tmp2, lsl #16

    sadd16 poly3, poly3, bias
    smulwb tmp, scale, poly3
    smulwt tmp2, scale, poly3
    pkhbt poly3, tmp, tmp2, lsl #16

    str.w poly0, [aptr], #4
    str.w poly1, [aptr], #4
    str.w poly2, [aptr], #4
    str.w poly3, [aptr], #4
    subs loop, #1
    bne 1b

    pop {r4-r8, pc}

.unreq aptr
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq tmp
.unreq tmp2
.unreq scale
.unreq bias
.unreq loop

// r=r+r<<1+r<<2+1
.align 2
.global poly_unround_asm
.type poly_unround_asm, %function
poly_unround_asm:
    push {r4-r10, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    poly7   .req r8
    tmp     .req r9
    scale   .req r10
    loop    .req r12

    movw scale, #1
    movt scale, #1
    movw loop, #32
1:
    ldr.w poly0, [aptr, #4*0]
    ldr.w poly1, [aptr, #4*1]
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]
    ldr.w poly4, [aptr, #4*4]
    ldr.w poly5, [aptr, #4*5]
    ldr.w poly6, [aptr, #4*6]
    ldr.w poly7, [aptr, #4*7]

    add tmp, poly0, poly0, lsl #1 // a0+a0*2=3a0
    add poly0, tmp, poly0, lsl #2 // 3a0+4a0=7a0
    add poly0, poly0, scale
    add tmp, poly1, poly1, lsl #1 
    add poly1, tmp, poly1, lsl #2 
    add poly1, poly1, scale
    add tmp, poly2, poly2, lsl #1 
    add poly2, tmp, poly2, lsl #2 
    add poly2, poly2, scale
    add tmp, poly3, poly3, lsl #1 
    add poly3, tmp, poly3, lsl #2 
    add poly3, poly3, scale
    add tmp, poly4, poly4, lsl #1 
    add poly4, tmp, poly4, lsl #2 
    add poly4, poly4, scale
    add tmp, poly5, poly5, lsl #1 
    add poly5, tmp, poly5, lsl #2 
    add poly5, poly5, scale
    add tmp, poly6, poly6, lsl #1 
    add poly6, tmp, poly6, lsl #2 
    add poly6, poly6, scale
    add tmp, poly7, poly7, lsl #1 
    add poly7, tmp, poly7, lsl #2 
    add poly7, poly7, scale

    str.w poly0, [aptr], #4
    str.w poly1, [aptr], #4
    str.w poly2, [aptr], #4
    str.w poly3, [aptr], #4
    str.w poly4, [aptr], #4
    str.w poly5, [aptr], #4
    str.w poly6, [aptr], #4
    str.w poly7, [aptr], #4
    subs loop, #1
    bne 1b

    pop {r4-r10, pc}

.unreq aptr 
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq poly4
.unreq poly5
.unreq poly6
.unreq poly7
.unreq tmp  
.unreq scale
.unreq loop


// r=r+r<<7+r<<8+1
// void poly_mul385(int16_t r[DIM_N]);
.align 2
.global poly_mul385_asm
.type poly_mul385_asm, %function
poly_mul385_asm:
    push {r4-r10, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    poly7   .req r8
    tmp     .req r9
    scale   .req r10
    loop    .req r12

    movw scale, #1
    movt scale, #1
    movw loop, #8
    .equ stride, 128*2
1:
    ldr.w poly0, [aptr, #4*0]
    ldr.w poly1, [aptr, #4*1]
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]
    ldr.w poly4, [aptr, #4*4]
    ldr.w poly5, [aptr, #4*5]
    ldr.w poly6, [aptr, #4*6]
    ldr.w poly7, [aptr, #4*7]

    add tmp, poly0, poly0, lsl #7 // a0+a0*128=129a0
    add poly0, tmp, poly0, lsl #8 // 129a0+256a0=385a0
    add tmp, poly1, poly1, lsl #7 
    add poly1, tmp, poly1, lsl #8 
    add tmp, poly2, poly2, lsl #7 
    add poly2, tmp, poly2, lsl #8 
    add tmp, poly3, poly3, lsl #7 
    add poly3, tmp, poly3, lsl #8 
    add tmp, poly4, poly4, lsl #7 
    add poly4, tmp, poly4, lsl #8 
    add tmp, poly5, poly5, lsl #7 
    add poly5, tmp, poly5, lsl #8 
    add tmp, poly6, poly6, lsl #7 
    add poly6, tmp, poly6, lsl #8 
    add tmp, poly7, poly7, lsl #7 
    add poly7, tmp, poly7, lsl #8 

    str.w poly1, [aptr, #4*1+stride*0]
    str.w poly1, [aptr, #4*1+stride*1]
    str.w poly1, [aptr, #4*1+stride*2]
    str.w poly1, [aptr, #4*1+stride*3]

    str.w poly2, [aptr, #4*2+stride*0]
    str.w poly2, [aptr, #4*2+stride*1]
    str.w poly2, [aptr, #4*2+stride*2]
    str.w poly2, [aptr, #4*2+stride*3]

    str.w poly3, [aptr, #4*3+stride*0]
    str.w poly3, [aptr, #4*3+stride*1]
    str.w poly3, [aptr, #4*3+stride*2]
    str.w poly3, [aptr, #4*3+stride*3]

    str.w poly4, [aptr, #4*4+stride*0]
    str.w poly4, [aptr, #4*4+stride*1]
    str.w poly4, [aptr, #4*4+stride*2]
    str.w poly4, [aptr, #4*4+stride*3]

    str.w poly5, [aptr, #4*5+stride*0]
    str.w poly5, [aptr, #4*5+stride*1]
    str.w poly5, [aptr, #4*5+stride*2]
    str.w poly5, [aptr, #4*5+stride*3]

    str.w poly6, [aptr, #4*6+stride*0]
    str.w poly6, [aptr, #4*6+stride*1]
    str.w poly6, [aptr, #4*6+stride*2]
    str.w poly6, [aptr, #4*6+stride*3]

    str.w poly7, [aptr, #4*7+stride*0]
    str.w poly7, [aptr, #4*7+stride*1]
    str.w poly7, [aptr, #4*7+stride*2]
    str.w poly7, [aptr, #4*7+stride*3]
    
    str.w poly0, [aptr, #4*0+stride*1]
    str.w poly0, [aptr, #4*0+stride*2]
    str.w poly0, [aptr, #4*0+stride*3]
    str.w poly0, [aptr], #4*8

    subs loop, #1
    bne 1b

    pop {r4-r10, pc}

.unreq aptr 
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq poly4
.unreq poly5
.unreq poly6
.unreq poly7
.unreq tmp  
.unreq scale
.unreq loop

//poly_xor4(int16_t r[128],int16_t a[512])
.align 2
.global poly_xor4_asm
.type poly_xor4_asm, %function
poly_xor4_asm:
    push {r4-r11, r14}

    rptr    .req r0
    aptr    .req r1
    poly0   .req r11
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    poly7   .req r8
    tmp     .req r9
    scale   .req r10
    tmp2    .req r14
    loop    .req r12

    movw scale, #1
    movt scale, #1
    movw loop, #32
    .equ stride, 128*2
1:
    ldr.w poly1, [aptr, #4+stride*0]
    ldr.w poly2, [aptr, #0+stride*1]
    ldr.w poly3, [aptr, #4+stride*1]
    ldr.w poly4, [aptr, #0+stride*2]
    ldr.w poly5, [aptr, #4+stride*2]
    ldr.w poly6, [aptr, #0+stride*3]
    ldr.w poly7, [aptr, #4+stride*3]
    ldr.w poly0, [aptr], #4*2

    eor tmp, poly0, poly2
    eor tmp2, poly4, poly6
    eor tmp, tmp, tmp2
    and poly0, tmp, scale

    eor tmp, poly1, poly3
    eor tmp2, poly5, poly7
    eor tmp, tmp, tmp2
    and poly1, tmp, scale

    str.w poly1, [rptr,#4]
    str.w poly0, [rptr], #4*2
    subs loop, #1
    bne 1b

    pop {r4-r11, pc}

.unreq aptr 
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq poly4
.unreq poly5
.unreq poly6
.unreq poly7
.unreq tmp  
.unreq scale
.unreq tmp2
.unreq loop 


//poly_unpack_f2(int16_t r[128],uint8_t a[16])
.align 2
.global poly_unpack_f2_asm
.type poly_unpack_f2_asm, %function
poly_unpack_f2_asm:
    push {r4-r11, r14}

    rptr    .req r0
    aptr    .req r1
    poly0   .req r11
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    poly7   .req r8
    tmp     .req r9
    scale   .req r10
    tmp2    .req r14
    loop    .req r12

    movw scale, #0x0101
    movt scale, #0x0101
    movw loop, #8

    ldr.w poly0, [aptr, #4*0]
    ldr.w poly1, [aptr, #4*1]
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]
    
    1:
    and tmp, scale, poly0 //r0|r1|r2|r3->r0|r1, r2|r3 and store back to memory
    uxtb poly4, tmp
    uxtb poly5, tmp, ror #8
    pkhbt poly4, poly4, poly5, lsl #16

    uxtb poly5, tmp, ror #16
    uxtb poly6, tmp, ror #24
    pkhbt poly5, poly5, poly6, lsl #16

    and tmp, scale, poly1 
    uxtb poly6, tmp
    uxtb poly7, tmp, ror #8
    pkhbt poly6, poly6, poly7, lsl #16

    uxtb poly7, tmp, ror #16
    uxtb tmp2, tmp, ror #24
    pkhbt poly7, poly7, tmp2, lsl #16

    str.w poly4, [rptr], #4
    str.w poly5, [rptr], #4
    str.w poly6, [rptr], #4
    str.w poly7, [rptr], #4

    and tmp, scale, poly2
    uxtb poly4, tmp
    uxtb poly5, tmp, ror #8
    pkhbt poly4, poly4, poly5, lsl #16

    uxtb poly5, tmp, ror #16
    uxtb poly6, tmp, ror #24
    pkhbt poly5, poly5, poly6, lsl #16

    and tmp, scale, poly3
    uxtb poly6, tmp
    uxtb poly7, tmp, ror #8
    pkhbt poly6, poly6, poly7, lsl #16

    uxtb poly7, tmp, ror #16
    uxtb tmp2, tmp, ror #24
    pkhbt poly7, poly7, tmp2, lsl #16

    str.w poly4, [rptr], #4
    str.w poly5, [rptr], #4
    str.w poly6, [rptr], #4
    str.w poly7, [rptr], #4

    lsr poly0, #1
    lsr poly1, #1
    lsr poly2, #1
    lsr poly3, #1

    subs loop, #1
    bne 1b

    pop {r4-r11, pc}

.unreq aptr 
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq poly4
.unreq poly5
.unreq poly6
.unreq poly7
.unreq tmp  
.unreq scale
.unreq tmp2
.unreq loop 


//poly_pack_f2(uint8_t a[16],int16_t r[128])
.align 2
.global poly_pack_f2_asm
.type poly_pack_f2_asm, %function
poly_pack_f2_asm:
    push {r4-r11, r14}

    rptr    .req r0
    aptr    .req r1
    poly0   .req r11
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    poly7   .req r8
    tmp     .req r9
    scale   .req r10
    tmp2    .req r14
    loop    .req r12

    movw loop, #8

    .equ stride, 16*2
    add aptr, #7*stride

    
    1:
    ldr.w poly0, [aptr, #4*0] //a0|0|a1|0
    ldr.w poly1, [aptr, #4*1] //a2|0|a3|0 -> a0|a1|a2|a3
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]

    uadd8 poly4, poly4, poly4
    uadd8 poly5, poly5, poly5
    uadd8 poly6, poly6, poly6
    uadd8 poly7, poly7, poly7

    orr tmp, poly0, poly0, lsr #8 //a0|a1|a1|0
    orr tmp2, poly1, poly1, lsr #8 //a2|a3|a3|0
    pkhbt poly0, tmp, tmp2, lsl #16 //a0|a1|a2|a3 
    uadd8 poly4, poly4, poly0

    orr tmp, poly2, poly2, lsr #8   
    orr tmp2, poly3, poly3, lsr #8  
    pkhbt poly1, tmp, tmp2, lsl #16 
    uadd8 poly5, poly5, poly1

    ldr.w poly0, [aptr, #4*4] //a0|a1
    ldr.w poly1, [aptr, #4*5] //a2|a3 -> a0|a1|a2|a3
    ldr.w poly2, [aptr, #4*6]
    ldr.w poly3, [aptr, #4*7]

    orr tmp, poly0, poly0, lsr #8 //a0|a1|a1|0
    orr tmp2, poly1, poly1, lsr #8 //a2|a3|a3|0
    pkhbt poly0, tmp, tmp2, lsl #16 //a0|a1|a2|a3 
    uadd8 poly6, poly6, poly0

    orr tmp, poly2, poly2, lsr #8   
    orr tmp2, poly3, poly3, lsr #8  
    pkhbt poly1, tmp, tmp2, lsl #16 
    uadd8 poly7, poly7, poly1

    sub aptr, aptr, #stride
    subs loop, #1
    bne 1b

    str.w poly4, [rptr, #4*0]
    str.w poly5, [rptr, #4*1]
    str.w poly6, [rptr, #4*2]
    str.w poly7, [rptr, #4*3]

    pop {r4-r11, pc}

.unreq aptr 
.unreq poly0
.unreq poly1
.unreq poly2
.unreq poly3
.unreq poly4
.unreq poly5
.unreq poly6
.unreq poly7
.unreq tmp  
.unreq scale
.unreq tmp2
.unreq loop 

.align 2
.global pointwise_add
.type pointwise_add, %function
pointwise_add:
  push {r4-r11, lr}

  movw r14, #51
  1:
    ldm r1!, {r3-r7}
    ldm r2!, {r8-r12}
    uadd16 r3, r3, r8
    uadd16 r4, r4, r9
    uadd16 r5, r5, r10
    uadd16 r6, r6, r11
    uadd16 r7, r7, r12
    stm r0!, {r3-r7}

    subs.w r14, #1
  bne.w 1b

  ldr.w r3, [r1], #4
  ldr.w r8, [r2], #4
  uadd16 r3, r3, r8
  str.w r3, [r0], #4
  pop {r4-r11, pc}


###################################
#### update cp and parity #########
#### r0: cp; r1: tmp2 #############
###################################
.align 2
.global update_cp_asm
.type update_cp_asm, %function
update_cp_asm:
    push {r4-r11, lr}

    cplow      .req r0
    lowptr     .req r1
    hiptr      .req r2
    outptr     .req r3
    cphigh     .req r4
    low        .req r5
    high       .req r6
    q          .req r7
    qa         .req r8
    tmp        .req r9
    plantconst .req r10
    ones       .req r11
    loop       .req r12
    parity     .req lr

    mov outptr, lowptr
    add.w hiptr, lowptr, #512
    add.w cphigh, cplow, #512
    movt q, #769
    movw qa, #24608
    movw plantconst, #14574
    movt plantconst, #85
    movw ones, #1
    movt ones, #1
    movw loop, #128

1:
    ldr.w low, [lowptr], #4
    ldr.w high, [hiptr], #4

    usub16 tmp, high, low
    uadd16 high, high, low

    doubleplant tmp, low, q, qa, plantconst
    doubleplant high, low, q, qa, plantconst

    and low, tmp, ones
    and parity, high, ones
    eor low, low, parity

    str.w tmp, [cplow], #4
    str.w high, [cphigh], #4    
    str.w low, [outptr], #4

    subs.w loop, #1
    bne.w 1b

    pop {r4-r11, pc}

 .unreq cplow
 .unreq lowptr
 .unreq hiptr
 .unreq outptr
 .unreq cphigh
 .unreq low
 .unreq high
 .unreq q
 .unreq qa
 .unreq tmp
 .unreq plantconst
 .unreq ones
 .unreq loop
 .unreq parity
