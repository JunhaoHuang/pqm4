

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "NTT_params.h"

#include "tools.h"
#include "gen_table.h"

#define BUFF_MAX (NTT_N << 3)

struct compress_profile profile;

int main()
{
    profile.compressed_layers = 3;
    profile.merged_layers[0] = 3;
    profile.merged_layers[1] = 3;
    profile.merged_layers[2] = 3;

    int32_t twiddle_int32[BUFF_MAX];

    int32_t scale_int32;
    int32_t omega_int32;
    int32_t mod_int32;
    
    scale_int32 = RmodQ1;
    omega_int32 = omegaQ1;
    mod_int32 = Q1;
    
    gen_streamlined_CT_negacyclic_table_generic(
        twiddle_int32,
        &scale_int32, &omega_int32,
        &mod_int32,
        sizeof(int32_t),
        mulmod_int32,
        &profile, 0);

    // we are going to have N twiddle factors
    printf("Twiddle factors for Raccoon Q1:");
    for (int i = 0; i < (NTT_N  - 1); i++)
    {
        printf("%d, ", (twiddle_int32[i]+mod_int32)%mod_int32);
    }
    printf("\n\n-------------------------------------\n\n");
    
    scale_int32 = RmodQ2;
    omega_int32 = omegaQ2;
    mod_int32 = Q2;

    gen_streamlined_CT_negacyclic_table_generic(
        twiddle_int32,
        &scale_int32, &omega_int32,
        &mod_int32,
        sizeof(int32_t),
        mulmod_int32,
        &profile, 0);

    // we are going to have N twiddle factors
    printf("Twiddle factors for Raccoon Q2:");
    for (int i = 0; i < (NTT_N - 1); i++)
    {
        printf("%d, ", (twiddle_int32[i] + mod_int32) % mod_int32);
    }
    printf("\n\n-------------------------------------\n\n");
    
    // inverse NTT
    scale_int32 = RmodQ1;
    omega_int32 = omegaQ1;
    mod_int32 = Q1;
    int32_t scale2_int32 = MONT_C4Q1;
    int32_t twist_omega_int32 = invomegaQ1;

    // (w^(-1))^2 mod Q1
    expmod_int32(&omega_int32, &twist_omega_int32, 2, &mod_int32);

    gen_streamlined_inv_CT_negacyclic_table_generic(
        twiddle_int32,
        &scale_int32, &omega_int32,
        &scale2_int32, &twist_omega_int32,
        &mod_int32,
        sizeof(int32_t),
        mulmod_int32,
        expmod_int32,
        &profile, 0);

    // we are going to have 512 inverse twiddle factors
    printf("Inverse Twiddle factors for Raccoon Q1:");
    for (int i = 0; i < (NTT_N << 1) - 1; i++)
    {
        printf("%d, ", twiddle_int32[i]);
    }
    printf("\n\n-------------------------------------\n\n");

    scale_int32 = RmodQ2;
    omega_int32 = omegaQ2;
    mod_int32 = Q2;
    scale2_int32 = MONT_C4Q2;
    twist_omega_int32 = invomegaQ2;

    // (w^(-1))^2 mod Q1
    expmod_int32(&omega_int32, &twist_omega_int32, 2, &mod_int32);

    gen_streamlined_inv_CT_negacyclic_table_generic(
        twiddle_int32,
        &scale_int32, &omega_int32,
        &scale2_int32, &twist_omega_int32,
        &mod_int32,
        sizeof(int32_t),
        mulmod_int32,
        expmod_int32,
        &profile, 0);

    // we are going to have 512 inverse twiddle factors
    printf("Inverse Twiddle factors for Raccoon Q2:");
    for (int i = 0; i < (NTT_N << 1) - 1; i++)
    {
        printf("%d, ", twiddle_int32[i]);
    }
    printf("\n\n-------------------------------------\n\n");
}
