#ifndef SMALLNTT_H
#define SMALLNTT_H

#include <stdint.h>
#include "params.h"

static const int16_t zetas[64] = {
-23, 112, -151, -134, -52, -148, 227, 232,
-71, 212, 236, 21, 341, 379, -202, -220,
352, 292, 238, 145, 194, -276, 70, -274,
117, 333, 66, 247, -237, -83, -252, -244,
331, -241, 167, 357, -355, 291, -358, 105, -115, -209, 14, 99, -260, 29, 366, -378, -318, 278, 353, 354, -184, 127, 330, -303, 222, -78, -348, -44, 201, 158, 350, 168
};

static const int16_t zetas_asm[128] = {
0, -164, -81, 361, 186, -3, -250, -120, -308, 129, -16, -223, -362, -143, 131, -337,
-76, 147, -114, -23, 112, -151, -134,
-98, -272, 54, -52, -148, 227, 232,
36, -2, -124, -71, 212, 236, 21,
-75, -80, -346, 341, 379, -202, -220,
-339, 86, -51, 352, 292, 238, 145,
-255, 364, 267, 194, -276, 70, -274,
282, 161, -15, 117, 333, 66, 247,
-203, 288, 169, -237, -83, -252, -244,
-34, 191, 307, 331, -241, 167, 357,
199, -50, -24, -355, 291, -358, 105,
178, -170, 226, -115, -209, 14, 99,
270, 121, -188, -260, 29, 366, -378,
-10, -380, 279, -318, 278, 353, 354,
149, 180, -375, -184, 127, 330, -303,
369, -157, 263, 222, -78, -348, -44,
-192, -128, -246, 201, 158, 350, 168
};

static const int16_t zetas_inv_CT_asm[256] = {
0, 171, 171, 164, 171, -361, 164, 81, 171, 120, -361, 3, 164, 250, 81, -186,
171, 164, 171, -361, 164, 81, -257, 49, -141, -18, -215, 38, 283, 347, 337, 192, -369, 246, -263, 128, 157, 239, -264, 179, 301, -207, 219, -332, -206, 120, 337, -131, 192, -149, -369, 10, 62, 57, 40, 136, 1, 311, -173, 27, 223, 203, -282, -169, 15, -288, -161, 74, -56, 271, -309, 26, -373, 116, -67, -361, 120, 250, 337, 143, -131, 362, -383, 82, 125, -344, -93, 299, -60, -204, 143, -270, -178, 188, -226, -121, 170, 39, -175, 174, 284, -111, 84, -22, 79, 3, 223, 16, 203, 255, -282, 339, 245, 64, -90, -306, 190, -123, 197, -253, -129, 75, -36, 346, 124, 80, 2, 218, 126, -33, -266, 326, -122, -261, 343, 164, -361, 81, 120, 3, 250, -186, 285, 200, -89, 5, 17, -96, 135, -310, -131, -149, 10, 375, -279, -180, 380, -280, -183, -7, 130, -327, -189, -335, -370, 250, 143, 362, -270, -199, -178, 34, -359, -144, -182, 304, -43, -300, -251, 377, 16, 255, 339, -267, 51, -364, -86, -106, 101, -118, 214, -349, -110, -374, -195, 81, 3, -186, 223, -129, 16, 308, 320, 319, 8, 181, 154, 216, 273, 313, 362, -199, 34, 24, -307, 50, -191, -139, -165, 208, 92, 159, 233, 177, -321, -186, -129, 308, 75, 98, -36, 76, 231, 324, 25, 85, 289, -94, -12, 113, 308, 98, 76, -54, 114, 272, -147, -146, -35, -119, -97, -176, -137, -312, -138,
};


#define SMALL_Q 769

void small_ntt_asm(int16_t a[N], const int16_t * zetas);
void small_invntt_tomont_asm(int16_t a[N], const int16_t * zetas);
void small_pointmul_asm(int16_t out[N], const int16_t in[N], const int16_t *zetas);
void small_asymmetric_mul_asm(int16_t c[256], const int16_t a[256], const int16_t b[256], const int16_t b_prime[256]);

#define small_ntt(a) small_ntt_asm(a, zetas_asm)
#define small_invntt_tomont(a) small_invntt_tomont_asm(a, zetas_inv_CT_asm)
#define small_point_mul(out, in) small_pointmul_asm(out, in, zetas)
#define small_asymmetric_mul(c, a, b, b_prime) small_asymmetric_mul_asm(c, a, b, b_prime);

#endif