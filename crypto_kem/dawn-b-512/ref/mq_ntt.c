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
  for(len = DIM_N >> 1; len >= 4; len >>= 1) 
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
  for(len = DIM_N >> 1; len >= 4; len >>= 1) 
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
    a[j] = fqmul(a[j], 1);
    a[j] += (a[j] >> 15) & Q;
  }
}

void mq_poly_intt(int16_t *a) 
{
  unsigned int start, len, j, k;
  int16_t t, zeta;

  k = 0;
  for(len = 4; len <= DIM_N >> 1; len <<= 1)
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
  int c0,  c1,  c2,  c3;
  c0 = fqmul(a[0], b[0]);
  c1 = fqmul(a[1], b[1]);
  c2 = fqmul(a[2], b[2]);
  c3 = fqmul(a[3], b[3]);

  r[0] = fqmul((a[1] + a[3]), (b[1] + b[3]));
  r[0] -= c1;
  r[0] -= c3;
  r[0] += c2;
  r[0] = fqmul(r[0], zeta);
  r[0] += c0;

  r[1] = fqmul((a[2] + a[3]), (b[2] + b[3]));
  r[1] -= c2;
  r[1] -= c3;
  r[1] = fqmul(r[1], zeta);
  r[1] += fqmul((a[0] + a[1]), (b[0] + b[1]));
  r[1] -= c0;
  r[1] -= c1;

  r[2] = fqmul(c3, zeta);
  r[2] += c1;
  r[2] += fqmul((a[0] + a[2]), (b[0] + b[2]));
  r[2] -= c0;
  r[2] -= c2;

  r[3] = fqmul((a[0] + a[3]), (b[0] + b[3]));
  r[3] -= c0;
  r[3] -= c3;
  r[3] += fqmul((a[1] + a[2]), (b[1] + b[2]));
  r[3] -= c1;
  r[3] -= c2;
}

void mq_poly_pointwise_mul(int16_t *r,  int16_t *a,  int16_t *b)
{
    int i;
    for(i = 0; i < DIM_N / 8; i++) 
    {
        base_mul(r + 8 * i, a + 8 * i, b + 8 * i, f[64 + i]);
        base_mul(r + 8 * i + 4, a + 8 * i + 4, b + 8 * i + 4, -f[64 + i]);
    }
}

void base_mul_mq(int16_t *r, int16_t *a, int16_t *b, int16_t zeta)
{
  int c0,  c1,  c2,  c3;
  c0 = fqmul(a[0], b[0]);
  c1 = fqmul(a[1], b[1]);
  c2 = fqmul(a[2], b[2]);
  c3 = fqmul(a[3], b[3]);

  r[0] = fqmul((a[1] + a[3]), (b[1] + b[3]));
  r[0] -= c1;
  r[0] -= c3;
  r[0] += c2;
  r[0] = fqmul(r[0], zeta);
  r[0] += c0;
  r[0] = fqmul(r[0], 1);
  r[0] += (r[0] >> 15) & Q;

  r[1] = fqmul((a[2] + a[3]), (b[2] + b[3]));
  r[1] -= c2;
  r[1] -= c3;
  r[1] = fqmul(r[1], zeta);
  r[1] += fqmul((a[0] + a[1]), (b[0] + b[1]));
  r[1] -= c0;
  r[1] -= c1;
  r[1] = fqmul(r[1], 1);
  r[1] += (r[1] >> 15) & Q;

  r[2] = fqmul(c3, zeta);
  r[2] += c1;
  r[2] += fqmul((a[0] + a[2]), (b[0] + b[2]));
  r[2] -= c0;
  r[2] -= c2;
  r[2] = fqmul(r[2], 1);
  r[2] += (r[2] >> 15) & Q;

  r[3] = fqmul((a[0] + a[3]), (b[0] + b[3]));
  r[3] -= c0;
  r[3] -= c3;
  r[3] += fqmul((a[1] + a[2]), (b[1] + b[2]));
  r[3] -= c1;
  r[3] -= c2;
  r[3] = fqmul(r[3], 1);
  r[3] += (r[3] >> 15) & Q;
}

void mq_poly_pointwise_mul_mq(int16_t *r,  int16_t *a,  int16_t *b)
{
    int i;
    for(i = 0; i < DIM_N / 8; i++) 
    {
        base_mul_mq(r + 8 * i, a + 8 * i, b + 8 * i, f[64 + i]);
        base_mul_mq(r + 8 * i + 4, a + 8 * i + 4, b + 8 * i + 4, -f[64 + i]);
    }
}

void base_inv(int16_t *r, int16_t *a, int16_t zeta)
{
  int t, k, det;
  int zeta2 = fqmul(zeta, zeta);

  r[0] = fqmul(a[2], a[2]);
  t = fqmul(a[1], a[3]);
  t = fqmul(t, 2);
  r[0] += t;
  r[0] = fqmul(r[0], a[0]);
  t = fqmul(a[1], a[1]);
  t = fqmul(t, a[2]);
  r[0] -= t;
  r[0] = fqmul(r[0], zeta);
  t = fqmul(a[3], a[3]);
  t = fqmul(t, a[2]);
  t = fqmul(t, zeta2);
  r[0] -= t;
  t = fqmul(a[0], a[0]);
  t = fqmul(t, a[0]);
  r[0] -= t;

  r[1] = fqmul(a[1], a[2]);
  t = fqmul(a[0], a[3]);
  t = fqmul(t, 2);
  r[1] -= t;
  r[1] = fqmul(r[1], a[2]);
  t = fqmul(a[1], a[1]);
  t = fqmul(t, a[3]);
  r[1] -= t;
  r[1] = fqmul(r[1], zeta);
  t = fqmul(a[3], a[3]);
  t = fqmul(t, a[3]);
  t = fqmul(t, zeta2);
  r[1] += t;
  t = fqmul(a[0], a[0]);
  t = fqmul(t, a[1]);
  r[1] += t;

  r[2] = fqmul(a[1], a[3]);
  r[2] = fqmul(r[2], 2);
  t = fqmul(a[2], a[2]);
  r[2] -= t;
  r[2] = fqmul(r[2], a[2]);
  t = fqmul(a[3], a[3]);
  t = fqmul(t, a[0]);
  r[2] -= t;
  r[2] = fqmul(r[2], zeta);
  t = fqmul(a[0], a[0]);
  t = fqmul(t, a[2]);
  r[2] += t;
  t = fqmul(a[1], a[1]);
  t = fqmul(t, a[0]);
  r[2] -= t;

  r[3] = fqmul(a[2], a[2]);
  t = fqmul(a[1], a[3]);
  r[3] -= t;
  r[3] = fqmul(r[3], a[3]);
  r[3] = fqmul(r[3], zeta);
  t = fqmul(a[1], a[1]);
  t = fqmul(t, a[1]);
  r[3] += t;
  t = fqmul(a[0], a[2]);
  t = fqmul(t, a[1]);
  t = fqmul(t, 2);
  r[3] -= t;
  t = fqmul(a[0], a[0]);
  t = fqmul(t, a[3]);
  r[3] += t;

  det = fqmul(a[2], a[2]);
  t = fqmul(a[1], a[3]);
  t = fqmul(t, 4);
  det -= t;
  det = fqmul(det, a[2]);
  det = fqmul(det, a[2]);
  t = fqmul(a[0], a[2]);
  t = fqmul(t, 2);
  t += fqmul(a[1], a[1]);
  t = fqmul(t, a[3]);
  t = fqmul(t, a[3]);
  t = fqmul(t, 2);
  det += t;
  det = fqmul(det, zeta2);
  t = fqmul(a[3], a[3]);
  t = fqmul(t, a[3]);
  t = fqmul(t, a[3]);
  t = fqmul(t, zeta2);
  t = fqmul(t, zeta);
  det = t-det;
  t = fqmul(a[0], a[0]);
  t = fqmul(t, a[0]);
  t = fqmul(t, a[0]);
  det -= t;
  t = fqmul(a[1], a[3]);
  t = fqmul(t, 2);
  t += fqmul(a[2], a[2]);
  t = fqmul(t, 2);
  t = fqmul(t, a[0]);
  t = fqmul(t, a[0]);
  k = fqmul(a[0], a[2]);
  k = fqmul(k, -4);
  k += fqmul(a[1], a[1]);
  k = fqmul(k, a[1]);
  k = fqmul(k, a[1]);
  t += k;
  t = fqmul(t, zeta);
  det += t;
  det = fqmul(det, 1);
  det += (det >> 15) & Q;
  det = qinv[det];

  r[0] = fqmul(r[0], det);
  r[1] = fqmul(r[1], det);
  r[2] = fqmul(r[2], det);
  r[3] = fqmul(r[3], det);
}

void mq_poly_inv_ntt(int16_t *r, int16_t *a)
{
  int i;
  for(i = 0; i < DIM_N / 8; i++) 
  {
    base_inv(r + 8 * i, a + 8 * i, f[64 + i]);
    base_inv(r + 8 * i + 4, a + 8 * i + 4, -f[64 + i]);
  }
}
