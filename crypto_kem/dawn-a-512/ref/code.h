#ifndef CODE_H
#define CODE_H
#include <stdio.h>
#include <stdint.h>

void encode_pk(int16_t *c, uint8_t *code_c);
void decode_pk(uint8_t *code_c, int16_t *c);
void encode_ct(int16_t *c, uint8_t *code_c);
void decode_ct(uint8_t *code_c, int16_t *c);
void encode_f(int16_t *c, uint8_t *code_c);
void decode_f(uint8_t *code_c, int16_t *c);


#endif