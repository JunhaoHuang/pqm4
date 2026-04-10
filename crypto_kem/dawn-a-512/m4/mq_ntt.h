#ifndef NTT_H
#define NTT_H

#include <stdint.h>
#include "param.h"

void mq_poly_ntt(int16_t *a);
void mq_poly_ntt_mq(int16_t *a);
void mq_poly_intt(int16_t *a);
void mq_poly_pointwise_mul(int16_t *r, int16_t *a, int16_t *b);
void mq_poly_pointwise_mul_mq(int16_t *r,  int16_t *a,  int16_t *b);
void mq_poly_inv_ntt(int16_t *r, int16_t *a);
#endif