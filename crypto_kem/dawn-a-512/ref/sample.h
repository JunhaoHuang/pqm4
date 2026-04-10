#ifndef SAMPLE_H
#define SAMPLE_H
#include <stdint.h>
#include "param.h"
#include "fips202.h"

void ternary_sample_fgk(int16_t *f, int16_t *g, uint8_t *k, shake256ctx *state);
void ternary_sample_f(int16_t *f, shake256ctx *state);
void ternary_sample_g(int16_t *g, shake256ctx *state);
void ternary_sample_se(int16_t *s, int16_t *e, shake256ctx *state);

#endif