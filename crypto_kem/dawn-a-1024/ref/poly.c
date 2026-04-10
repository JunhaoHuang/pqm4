#include <stdint.h>
#include <stdio.h>
#include "param.h"

int16_t check_poly_inv_Zq(int16_t *a)
{
	int i, j, flag;
    for(i = 0; i < DIM_N; i += 8)
	{
		flag = a[i] + a[i + 1] + a[i + 2] + a[i + 3] + a[i + 4] + a[i + 5] + a[i + 6] + a[i + 7];
		if(flag == 0)
			return 1;
	}
	return 0;
}

int16_t check_poly_inv_Z2(int16_t *a)
{
	int16_t i, t = 0;
    for(i = 0; i < N_2; i++)
    {
        t ^= a[i];
    }
    if(t)
        return 0;

	return 1;
}

void poly_round(int16_t *a)
{
    int i;
    for(i = 0; i < DIM_N; i++)
    {
		a[i] = a[i] >> 2;
    }
}