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
    push {r4-r11, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r7
    poly5   .req r8
    poly6   .req r9
    poly7   .req r10
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
    ldr.w poly4, [aptr], #4
    ldr.w poly5, [aptr], #4
    ldr.w poly6, [aptr], #4
    ldr.w poly7, [aptr], #4

    smuad tmp, poly0, ones
    smlad tmp, poly1, ones, tmp
    smlad tmp, poly2, ones, tmp
    smlad tmp, poly3, ones, tmp
    cbz tmp, 2f

    smuad tmp, poly4, ones
    smlad tmp, poly5, ones, tmp
    smlad tmp, poly6, ones, tmp
    smlad tmp, poly7, ones, tmp
    cbz tmp, 2f
    subs loop, #1
    bne 1b

    movs r0, #0
    pop {r4-r11, pc}

2:
    movs r0, #1
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
    movw tmp3, #16*2

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
    push {r4-r11, lr}

    aptr    .req r0
    poly0   .req r1
    poly1   .req r2
    poly2   .req r3
    poly3   .req r4
    poly4   .req r5
    poly5   .req r6
    poly6   .req r7
    loop    .req r8
    poly7   .req r12
    scale   .req r9

    movw scale, #0x3fff // mask the low 14-bit; eliminate the 2-bit from top-half
    movt scale, #0x3fff
    movw loop, #64
1:
    ldr.w poly0, [aptr, #0]
    ldr.w poly1, [aptr, #4]
    ldr.w poly2, [aptr, #8]
    ldr.w poly3, [aptr, #12]
    ldr.w poly4, [aptr, #4*4]
    ldr.w poly5, [aptr, #4*5]
    ldr.w poly6, [aptr, #4*6]
    ldr.w poly7, [aptr, #4*7]

    and poly0, scale, poly0, lsr #2
    and poly1, scale, poly1, lsr #2
    and poly2, scale, poly2, lsr #2
    and poly3, scale, poly3, lsr #2
    and poly4, scale, poly4, lsr #2
    and poly5, scale, poly5, lsr #2
    and poly6, scale, poly6, lsr #2
    and poly7, scale, poly7, lsr #2

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

    pop {r4-r11, pc}

.unreq aptr  
.unreq poly0 
.unreq poly1 
.unreq poly2 
.unreq poly3 
.unreq poly4 
.unreq poly5 
.unreq poly6 
.unreq loop  
.unreq poly7 
.unreq scale 

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
    movw loop, #32*2
1:
    ldr.w poly0, [aptr, #4*0]
    ldr.w poly1, [aptr, #4*1]
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]
    ldr.w poly4, [aptr, #4*4]
    ldr.w poly5, [aptr, #4*5]
    ldr.w poly6, [aptr, #4*6]
    ldr.w poly7, [aptr, #4*7]

    lsl poly0, poly0, #2
    lsl poly1, poly1, #2
    lsl poly2, poly2, #2
    lsl poly3, poly3, #2
    lsl poly4, poly4, #2
    lsl poly5, poly5, #2
    lsl poly6, poly6, #2
    lsl poly7, poly7, #2

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
    movw loop, #8*2
    .equ stride, 128*2*2
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

//poly_xor4(int16_t r[256],int16_t a[1024])
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
    movw loop, #32*2
    .equ stride, 128*2*2
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


//poly_unpack_f2(int16_t r[256],uint8_t a[32])
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
    ldr.w poly4, [aptr, #4*4]
    ldr.w poly5, [aptr, #4*5]
    ldr.w poly6, [aptr, #4*6]
    ldr.w poly7, [aptr, #4*7]
    vmov s0, poly0
    vmov s1, poly1
    vmov s2, poly2
    vmov s3, poly3
    vmov s4, poly4
    vmov s5, poly5
    vmov s6, poly6
    vmov s7, poly7
    1:
    vmov poly0, s0
    vmov poly1, s1
    vmov poly2, s2
    vmov poly3, s3
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
    
    vmov s0, poly0
    vmov s1, poly1
    vmov s2, poly2
    vmov s3, poly3

    // next 16 bytes
    vmov poly0, s4
    vmov poly1, s5
    vmov poly2, s6
    vmov poly3, s7
    
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
    
    vmov s4, poly0
    vmov s5, poly1
    vmov s6, poly2
    vmov s7, poly3

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


//poly_pack_f2(uint8_t a[32],int16_t r[256])
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

    .equ stride, 32*2
    add aptr, #7*stride

    uadd8 poly4, poly4, poly4
    vmov s0, poly4
    vmov s1, poly4
    vmov s2, poly4
    vmov s3, poly4
    vmov s4, poly4
    vmov s5, poly4
    vmov s6, poly4
    vmov s7, poly4
    
    1:
    ldr.w poly0, [aptr, #4*0] //a0|0|a1|0
    ldr.w poly1, [aptr, #4*1] //a2|0|a3|0 -> a0|a1|a2|a3
    ldr.w poly2, [aptr, #4*2]
    ldr.w poly3, [aptr, #4*3]

    vmov poly4, s0
    vmov poly5, s1
    vmov poly6, s2
    vmov poly7, s3

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

    vmov s0, poly4
    vmov s1, poly5
    vmov s2, poly6
    vmov s3, poly7

    // next 16-byte
    ldr.w poly0, [aptr, #4*8] //a0|0|a1|0
    ldr.w poly1, [aptr, #4*9] //a2|0|a3|0 -> a0|a1|a2|a3
    ldr.w poly2, [aptr, #4*10]
    ldr.w poly3, [aptr, #4*11]

    vmov poly4, s4
    vmov poly5, s5
    vmov poly6, s6
    vmov poly7, s7
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

    ldr.w poly0, [aptr, #4*12] //a0|a1
    ldr.w poly1, [aptr, #4*13] //a2|a3 -> a0|a1|a2|a3
    ldr.w poly2, [aptr, #4*14]
    ldr.w poly3, [aptr, #4*15]

    orr tmp, poly0, poly0, lsr #8 //a0|a1|a1|0
    orr tmp2, poly1, poly1, lsr #8 //a2|a3|a3|0
    pkhbt poly0, tmp, tmp2, lsl #16 //a0|a1|a2|a3 
    uadd8 poly6, poly6, poly0

    orr tmp, poly2, poly2, lsr #8   
    orr tmp2, poly3, poly3, lsr #8  
    pkhbt poly1, tmp, tmp2, lsl #16 
    uadd8 poly7, poly7, poly1

    vmov s4, poly4
    vmov s5, poly5
    vmov s6, poly6
    vmov s7, poly7

    sub aptr, aptr, #stride
    subs loop, #1
    bne 1b
    vmov poly0, s0
    vmov poly1, s1
    vmov poly2, s2
    vmov poly3, s3
    vmov poly4, s4
    vmov poly5, s5
    vmov poly6, s6
    vmov poly7, s7
    str.w poly0, [rptr, #4*0]
    str.w poly1, [rptr, #4*1]
    str.w poly2, [rptr, #4*2]
    str.w poly3, [rptr, #4*3]
    str.w poly4, [rptr, #4*4]
    str.w poly5, [rptr, #4*5]
    str.w poly6, [rptr, #4*6]
    str.w poly7, [rptr, #4*7]

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

  movw r14, #51*2
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
  ldr.w r4, [r1], #4
  ldr.w r8, [r2], #4
  ldr.w r9, [r2], #4
  uadd16 r3, r3, r8
  uadd16 r4, r4, r9
  str.w r3, [r0], #4
  str.w r4, [r0], #4
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
    add.w hiptr, lowptr, #512*2
    add.w cphigh, cplow, #512*2
    movt q, #769
    movw qa, #24608
    movw plantconst, #14574
    movt plantconst, #85
    movw ones, #1
    movt ones, #1
    movw loop, #128*2

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
