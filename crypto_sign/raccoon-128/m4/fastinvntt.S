.syntax unified
#include "macros.i"

// void raccoon_ntt(int32_t p[N], int32_t zeta[N], int32_t qqinv[2])
.global raccoon_invntt
.type raccoon_invntt,%function
.align 2
raccoon_invntt: