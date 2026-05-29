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

// Mul_in_R2_n_asm(int16_t *r, const int16_t *a, const int16_t *b, int16_t n)
// Computes r[j] = xor_{i: a[i] != 0} b[(j - i) mod n], 0 <= j < n.
// This is the unmasked sparse version: it skips the inner loops when a[i] == 0.
// n is assumed to be non-zero and even number. The DAWN call sites use even powers of two.
// output: r[n]; input a[n]; b[2*n]; n
.align 2
.global Mul_in_R2_2_asm
.type Mul_in_R2_2_asm, %function
Mul_in_R2_2_asm:
    push {r4-r11, r14}
    rptr    .req r0
    aptr    .req r1
    bptr    .req r2
	num     .req r3
    poly0   .req r14
    poly1   .req r4
    poly2   .req r5
    poly3   .req r6
    poly4   .req r7
    poly5   .req r8
    tmp     .req r9
    scale   .req r10
	idx     .req r11
    loop    .req r12

    movw scale, #0xffff
    lsr loop, num, #1
    vmov s0, rptr

    1:
    ldr.w poly0, [aptr], #4 // 2 ai at a time
    
    lowhalf:
    add poly1, bptr, loop, lsl #2 //bptr+2*(n-i) calculate index of b; retrieve in highhalf
    vmov s1, poly1

    and poly2, scale, poly0
    cbz poly2, highhalf

    lsr idx, num, #1
    vmov rptr, s0
    2: // xor loop
    ldr.w poly2, [poly1], #4
    ldr.w poly4, [rptr, #0]

    eor poly4, poly4, poly2
    
    str.w poly4, [rptr], #4
    subs idx, #1
    bne 2b

    highhalf:
    vmov rptr, s0
    vmov poly1, s1
    sub poly1, poly1, #2 //sub 1

    and poly2, scale, poly0, lsr #16
    cbz poly2, end
    lsr idx, num, #1

    3: // xor loop
    ldr.w poly2, [poly1], #4
    ldr.w poly4, [rptr, #0]

    eor poly4, poly4, poly2
    
    str.w poly4, [rptr], #4
    subs idx, #1
    bne 3b

    end:
    subs loop, #1
    bne 1b

	pop {r4-r11, pc}


.align 2
.global Mul_in_R2_n_asm
.type Mul_in_R2_n_asm, %function
Mul_in_R2_n_asm:
    push {r4-r11, r14}
    rptr    .req r0
    aptr    .req r1
    bptr    .req r2
	num     .req r3
    poly0   .req r14
    poly1   .req r4
    poly2   .req r5
    poly3   .req r6
    poly4   .req r7
    poly5   .req r8
    tmp     .req r9
    scale   .req r10
	idx     .req r11
    loop    .req r12

    movw scale, #0xffff
    lsr loop, num, #1
    vmov s0, rptr

    1:
    ldr.w poly0, [aptr], #4 // 2 ai at a time
    
    lowhalf1:
    add poly1, bptr, loop, lsl #2 //bptr+2*(n-i) calculate index of b; retrieve in highhalf
    vmov s1, poly1

    and poly2, scale, poly0
    cbz poly2, highhalf1

    lsr idx, num, #2
    vmov rptr, s0
    2: // xor loop
    ldr.w poly2, [poly1], #4
    ldr.w poly3, [poly1], #4
    ldr.w poly4, [rptr, #0]
    ldr.w poly5, [rptr, #4]

    eor poly4, poly4, poly2
    eor poly5, poly5, poly3
    
    str.w poly4, [rptr], #4
    str.w poly5, [rptr], #4
    subs idx, #1
    bne 2b

    highhalf1:
    vmov rptr, s0
    vmov poly1, s1
    sub poly1, poly1, #2 //sub 1

    and poly2, scale, poly0, lsr #16
    cbz poly2, end1
    lsr idx, num, #2

    3: // xor loop
    ldr.w poly2, [poly1], #4
    ldr.w poly3, [poly1], #4
    ldr.w poly4, [rptr, #0]
    ldr.w poly5, [rptr, #4]

    eor poly4, poly4, poly2
    eor poly5, poly5, poly3
    
    str.w poly4, [rptr], #4
    str.w poly5, [rptr], #4
    subs idx, #1
    bne 3b

    end1:
    subs loop, #1
    bne 1b

	pop {r4-r11, pc}
