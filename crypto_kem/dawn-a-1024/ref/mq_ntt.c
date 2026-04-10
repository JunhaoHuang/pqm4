#include <stdint.h>
#include <stdio.h>
#include "param.h"
#include "mq_ntt.h"
#include "mq_ntt_param.h"

inline int16_t montgomery_reduce(int32_t a)
{
  int32_t t;
  int16_t u;

  u = a * QINV;
  t = (int32_t)u * Q;
  t = a - t;
  t >>= 16;
  return t;
}

inline int16_t fqmul(int16_t a, int16_t b) 
{
  return montgomery_reduce((int32_t)a * b);
}


void mq_poly_ntt(int16_t *a) 
{
  unsigned int len, start, j, k;
  int16_t t, zeta;

  k = 1;
  for(len = DIM_N >> 1; len >= 8; len >>= 1) 
  {
    for(start = 0; start < DIM_N; start = j + len) 
    {
      zeta = f[k++];
      for(j = start; j < start + len; j++) 
      {
        t = fqmul(zeta, a[j + len]);
        a[j + len] = a[j] - t;
        a[j] = a[j] + t;
      }
    }
  }
}

void mq_poly_ntt_mq(int16_t *a) 
{
  unsigned int len, start, j, k;
  int16_t t, zeta;

  k = 1;
  for(len = DIM_N >> 1; len >= 8; len >>= 1) 
  {
    for(start = 0; start < DIM_N; start = j + len) 
    {
      zeta = f[k++];
      for(j = start; j < start + len; j++) 
      {
        t = fqmul(zeta, a[j + len]);
        a[j + len] = a[j] - t;
        a[j] = a[j] + t;
      }
    }
  }

  for(j = 0; j < DIM_N; j++)
  {
    a[j] = fqmul(a[j], 171);
    a[j] += (a[j] >> 15) & Q;
  }
}

void mq_poly_intt(int16_t *a) 
{
  unsigned int start, len, j, k;
  int16_t t, zeta;

  k = 0;
  for(len = 8; len <= DIM_N >> 1; len <<= 1)
  {
    for(start = 0; start < DIM_N; start =j + len)
    {
      zeta = fn[k++];
      for(j = start; j < start + len; j++) 
      {
        t = a[j];
        a[j] = (t + a[j + len]);
        a[j + len] = t - a[j + len];
        a[j + len] = fqmul(zeta, a[j + len]);
      }
    }
  }

  for(j = 0; j < DIM_N; j++)
  {
    a[j] = fqmul(a[j], fn[127]);
    // a[j] += (a[j] >> 15) & Q;
  }
}

void base_mul(int16_t *r, int16_t *a, int16_t *b, int16_t zeta)
{
  int16_t k;
  r[0] = fqmul(a[0], b[0]);
  k = fqmul(a[7], b[1]);
  k += fqmul(a[6], b[2]);
  k += fqmul(a[5], b[3]);
  k += fqmul(a[4], b[4]);
  k += fqmul(a[3], b[5]);
  k += fqmul(a[2], b[6]);
  k += fqmul(a[1], b[7]);
  r[0] += fqmul(k, zeta);

  r[1] = fqmul(a[1], b[0]);
  r[1] += fqmul(a[0], b[1]);
  k = fqmul(a[7], b[2]);
  k += fqmul(a[6], b[3]);
  k += fqmul(a[5], b[4]);
  k += fqmul(a[4], b[5]);
  k += fqmul(a[3], b[6]);
  k += fqmul(a[2], b[7]);
  r[1] += fqmul(k, zeta);

  r[2] = fqmul(a[2], b[0]);
  r[2] += fqmul(a[1], b[1]);
  r[2] += fqmul(a[0], b[2]);
  k = fqmul(a[7], b[3]);
  k += fqmul(a[6], b[4]);
  k += fqmul(a[5], b[5]);
  k += fqmul(a[4], b[6]);
  k += fqmul(a[3], b[7]);
  r[2] += fqmul(k, zeta);

  r[3] = fqmul(a[3], b[0]);
  r[3] += fqmul(a[2], b[1]);
  r[3] += fqmul(a[1], b[2]);
  r[3] += fqmul(a[0], b[3]);
  k = fqmul(a[7], b[4]);
  k += fqmul(a[6], b[5]);
  k += fqmul(a[5], b[6]);
  k += fqmul(a[4], b[7]);
  r[3] += fqmul(k, zeta);

  r[4] = fqmul(a[4], b[0]);
  r[4] += fqmul(a[3], b[1]);
  r[4] += fqmul(a[2], b[2]);
  r[4] += fqmul(a[1], b[3]);
  r[4] += fqmul(a[0], b[4]);
  k = fqmul(a[7], b[5]);
  k += fqmul(a[6], b[6]);
  k += fqmul(a[5], b[7]);
  r[4] += fqmul(k, zeta);

  r[5] = fqmul(a[5], b[0]);
  r[5] += fqmul(a[4], b[1]);
  r[5] += fqmul(a[3], b[2]);
  r[5] += fqmul(a[2], b[3]);
  r[5] += fqmul(a[1], b[4]);
  r[5] += fqmul(a[0], b[5]);
  k = fqmul(a[7], b[6]);
  k += fqmul(a[6], b[7]);
  r[5] += fqmul(k, zeta);

  k = fqmul(a[7], b[7]);
  r[6]=fqmul(k, zeta);
  r[6] += fqmul(a[6], b[0]);
  r[6] += fqmul(a[5], b[1]);
  r[6] += fqmul(a[4], b[2]);
  r[6] += fqmul(a[3], b[3]);
  r[6] += fqmul(a[2], b[4]);
  r[6] += fqmul(a[1], b[5]);
  r[6] += fqmul(a[0], b[6]);

  r[7] = fqmul(a[7], b[0]);
  r[7] += fqmul(a[6], b[1]);
  r[7] += fqmul(a[5], b[2]);
  r[7] += fqmul(a[4], b[3]);
  r[7] += fqmul(a[3], b[4]);
  r[7] += fqmul(a[2], b[5]);
  r[7] += fqmul(a[1], b[6]);
  r[7] += fqmul(a[0], b[7]);
}

void mq_poly_pointwise_mul(int16_t *r,  int16_t *a,  int16_t *b)
{
  int i;
  for(i = 0; i < DIM_N / 16; i++) 
  {
    base_mul(r + 16 * i, a + 16 * i, b + 16 * i, f[64 + i]);
    base_mul(r + 16 * i + 8, a + 16 * i + 8, b + 16 * i + 8, -f[64 + i]);
  }
}

void base_mul_mq(int16_t *r, int16_t *a, int16_t *b, int16_t zeta)
{
    int16_t k;
  r[0] = fqmul(a[0], b[0]);
  k = fqmul(a[7], b[1]);
  k += fqmul(a[6], b[2]);
  k += fqmul(a[5], b[3]);
  k += fqmul(a[4], b[4]);
  k += fqmul(a[3], b[5]);
  k += fqmul(a[2], b[6]);
  k += fqmul(a[1], b[7]);
  r[0] += fqmul(k, zeta);
  r[0] = fqmul(r[0], 19);
  r[0] += (r[0] >> 15) & Q;

  r[1] = fqmul(a[1], b[0]);
  r[1] += fqmul(a[0], b[1]);
  k = fqmul(a[7], b[2]);
  k += fqmul(a[6], b[3]);
  k += fqmul(a[5], b[4]);
  k += fqmul(a[4], b[5]);
  k += fqmul(a[3], b[6]);
  k += fqmul(a[2], b[7]);
  r[1] += fqmul(k, zeta);
  r[1] = fqmul(r[1], 19);
  r[1] += (r[1] >> 15) & Q;

  r[2] = fqmul(a[2], b[0]);
  r[2] += fqmul(a[1], b[1]);
  r[2] += fqmul(a[0], b[2]);
  k = fqmul(a[7], b[3]);
  k += fqmul(a[6], b[4]);
  k += fqmul(a[5], b[5]);
  k += fqmul(a[4], b[6]);
  k += fqmul(a[3], b[7]);
  r[2] += fqmul(k, zeta);
  r[2] = fqmul(r[2], 19);
  r[2] += (r[2] >> 15) & Q;

  r[3] = fqmul(a[3], b[0]);
  r[3] += fqmul(a[2], b[1]);
  r[3] += fqmul(a[1], b[2]);
  r[3] += fqmul(a[0], b[3]);
  k = fqmul(a[7], b[4]);
  k += fqmul(a[6], b[5]);
  k += fqmul(a[5], b[6]);
  k += fqmul(a[4], b[7]);
  r[3] += fqmul(k, zeta);
  r[3] = fqmul(r[3], 19);
  r[3] += (r[3] >> 15) & Q;

  r[4] = fqmul(a[4], b[0]);
  r[4] += fqmul(a[3], b[1]);
  r[4] += fqmul(a[2], b[2]);
  r[4] += fqmul(a[1], b[3]);
  r[4] += fqmul(a[0], b[4]);
  k = fqmul(a[7], b[5]);
  k += fqmul(a[6], b[6]);
  k += fqmul(a[5], b[7]);
  r[4] += fqmul(k, zeta);
  r[4] = fqmul(r[4], 19);
  r[4] += (r[4] >> 15) & Q;

  r[5] = fqmul(a[5], b[0]);
  r[5] += fqmul(a[4], b[1]);
  r[5] += fqmul(a[3], b[2]);
  r[5] += fqmul(a[2], b[3]);
  r[5] += fqmul(a[1], b[4]);
  r[5] += fqmul(a[0], b[5]);
  k = fqmul(a[7], b[6]);
  k += fqmul(a[6], b[7]);
  r[5] += fqmul(k, zeta);
  r[5] = fqmul(r[5], 19);
  r[5] += (r[5] >> 15) & Q;

  k = fqmul(a[7], b[7]);
  r[6]=fqmul(k, zeta);
  r[6] += fqmul(a[6], b[0]);
  r[6] += fqmul(a[5], b[1]);
  r[6] += fqmul(a[4], b[2]);
  r[6] += fqmul(a[3], b[3]);
  r[6] += fqmul(a[2], b[4]);
  r[6] += fqmul(a[1], b[5]);
  r[6] += fqmul(a[0], b[6]);
  r[6] = fqmul(r[6], 19);
  r[6] += (r[6] >> 15) & Q;

  r[7] = fqmul(a[7], b[0]);
  r[7] += fqmul(a[6], b[1]);
  r[7] += fqmul(a[5], b[2]);
  r[7] += fqmul(a[4], b[3]);
  r[7] += fqmul(a[3], b[4]);
  r[7] += fqmul(a[2], b[5]);
  r[7] += fqmul(a[1], b[6]);
  r[7] += fqmul(a[0], b[7]);
  r[7] = fqmul(r[7], 19);
  r[7] += (r[7] >> 15) & Q;
}

void mq_poly_pointwise_mul_mq(int16_t *r,  int16_t *a,  int16_t *b)
{
  int i;
  for(i = 0; i < DIM_N / 16; i++) 
  {
    base_mul_mq(r + 16 * i, a + 16 * i, b + 16 * i, f[64 + i]);
    base_mul_mq(r + 16 * i + 8, a + 16 * i + 8, b + 16 * i + 8, -f[64 + i]);
  }
}

void base_inv(int16_t *r, int16_t *a, int16_t zeta)
{
  int16_t t, k;
  int16_t b[4], c[2], e, f[4]; 

  b[0] = fqmul(a[1], a[7]);
  t = fqmul(a[3], a[5]);
  b[0] += t;
  t = fqmul(a[2], a[6]);
  b[0] -= t;
  b[0] = fqmul(b[0], 342); //2
  t = fqmul(a[4], a[4]);
  b[0] -= t;
  b[0] = fqmul(b[0], zeta);
  t = fqmul(a[0], a[0]);
  b[0] += t;

  b[1] = fqmul(a[3], a[7]);
  t = fqmul(a[4], a[6]);
  b[1] -= t;
  b[1] = fqmul(b[1], 342); //2
  t = fqmul(a[5], a[5]);
  b[1] += t;
  b[1] = fqmul(b[1], zeta);
  t = fqmul(a[0], a[2]);
  t = fqmul(t, 342); //2
  b[1] += t;
  t = fqmul(a[1], a[1]);
  b[1] -= t;

  b[2] = fqmul(a[5], a[7]);
  b[2] = fqmul(b[2], 342); //2
  t = fqmul(a[6], a[6]);
  b[2] -= t;
  b[2] = fqmul(b[2], zeta);
  t = fqmul(a[0], a[4]);
  k = fqmul(a[1], a[3]);
  t -= k;
  t = fqmul(t, 342); //2
  b[2] += t;
  t = fqmul(a[2], a[2]);
  b[2] += t;

  b[3] = fqmul(a[7], a[7]);
  b[3] = fqmul(b[3], zeta);
  t = fqmul(a[0], a[6]);
  k = fqmul(a[2], a[4]);
  t += k;
  k = fqmul(a[1], a[5]);
  t -= k;
  t = fqmul(t, 342); //2
  b[3] += t;
  t = fqmul(a[3], a[3]);
  b[3] -= t;

  c[0] = fqmul(b[1], b[3]);
  c[0] = fqmul(c[0], 342); //2
  t = fqmul(b[2], b[2]);
  c[0] -= t;
  c[0] = fqmul(c[0], zeta);
  t = fqmul(b[0], b[0]);
  c[0] += t;

  c[1] = fqmul(b[3], b[3]);
  c[1] = fqmul(c[1], zeta);
  t = fqmul(b[0], b[2]);
  t = fqmul(t, 342); //2
  c[1] += t;
  t = fqmul(b[1], b[1]);
  c[1] -= t;

  e = fqmul(c[1], c[1]);
  e = fqmul(e, zeta);
  t = fqmul(c[0], c[0]);
  e += t;
  e += (e >> 15) & Q;
  e = qinv[e];

  c[0] = fqmul(e, c[0]);

  c[1] = fqmul(e, c[1]);
  c[1] = fqmul(c[1], -171);

  f[0] = fqmul(c[1], b[2]);
  f[0] = fqmul(f[0], zeta);
  t = fqmul(c[0], b[0]);
  f[0] = t- f[0];

  f[1] = fqmul(c[1], b[3]);
  f[1] = fqmul(f[1], zeta);
  t = fqmul(c[0], b[1]);
  f[1] -= t;

  f[2] = fqmul(c[0], b[2]);
  t = fqmul(c[1], b[0]);
  f[2] += t;

  f[3] = fqmul(c[0], b[3]);
  t = fqmul(c[1], b[1]);
  f[3] += t;
  f[3] = fqmul(f[3], -171);

  r[0] = fqmul(f[1], a[6]);
  t = fqmul(f[2], a[4]);
  r[0] += t;
  t = fqmul(f[3], a[2]);
  r[0] += t;
  r[0] = fqmul(r[0], zeta);
  t = fqmul(f[0], a[0]);
  r[0] = t - r[0];

  r[1] = fqmul(f[1], a[7]);
  t = fqmul(f[2], a[5]);
  r[1] += t;
  t  = fqmul(f[3], a[3]);
  r[1] += t;
  r[1] = fqmul(r[1], zeta);
  t = fqmul(f[0], a[1]);
  r[1] -= t;

  r[2] = fqmul(f[2], a[6]);
  t = fqmul(f[3], a[4]);
  r[2] += t;
  r[2] = fqmul(r[2], zeta);
  t = fqmul(f[0], a[2]);
  k = fqmul(f[1], a[0]);
  t += k;
  r[2] = t - r[2];

  r[3] = fqmul(f[2], a[7]);
  t = fqmul(f[3], a[5]);
  r[3] += t;
  r[3] = fqmul(r[3], zeta);
  t = fqmul(f[0], a[3]);
  k = fqmul(f[1], a[1]);
  t += k;
  r[3] -= t;

  r[4] = fqmul(f[3], a[6]);
  r[4] = fqmul(r[4], zeta);
  t = fqmul(f[0], a[4]);
  k = fqmul(f[1], a[2]);
  t += k;
  k = fqmul(f[2], a[0]);
  t += k;
  r[4] = t - r[4];

  r[5] = fqmul(f[3], a[7]);
  r[5] = fqmul(r[5], zeta);
  t = fqmul(f[0], a[5]);
  k = fqmul(f[1], a[3]);
  t += k;
  k = fqmul(f[2], a[1]);
  t += k;
  r[5] -= t;

  r[6] = fqmul(f[1], a[4]);
  t = fqmul(f[2], a[2]);
  r[6] += t;
  t = fqmul(f[3], a[0]);
  r[6] += t;
  t = fqmul(f[0], a[6]);
  r[6] += t;

  r[7] = fqmul(f[0], a[7]);
  t = fqmul(f[1], a[5]);
  r[7] += t;
  t = fqmul(f[2], a[3]);
  r[7] += t;
  t = fqmul(f[3], a[1]);
  r[7] += t;
  r[7] = fqmul(r[7], -171);

}

void mq_poly_inv_ntt(int16_t *r, int16_t *a)
{
  int i;
  for(i = 0; i < DIM_N / 16; i++) 
  {
    base_inv(r + 16 * i, a + 16 * i, -f[64 + i]);
    base_inv(r + 16 * i + 8, a + 16 * i + 8, f[64 + i]);
  }
}