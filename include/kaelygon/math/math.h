/**
 * @file math.h
 * 
 * @brief Implementation, Strictly unsigned arithmetic
 */
#pragma once

#include <stdint.h>
#include <limits.h>
#include "kaelygon/global/kaelMacros.h"

//kaelmath

uint16_t kaelMath_min(uint16_t a, uint16_t b);
uint16_t kaelMath_max(uint16_t a, uint16_t b);
uint16_t kaelMath_isNegative(uint16_t a);
uint16_t kaelMath_gtZeroSigned(uint16_t a);
uint16_t kaelMath_abs(uint16_t a);
uint16_t kaelMath_sub(uint16_t a, uint16_t b);

uint8_t kaelMath_log10(uint16_t num);

uint16_t kaelMath_rorr(uint16_t num, const uint16_t shift);
uint8_t kaelMath_u8rorr(uint8_t num, uint8_t shift);

uint16_t kaelRand_lcg(uint16_t seed);

// 3 = _rand24States, it's not possible to have this non-global variable or define so we use magic number 
uint8_t kaelRand_lcg24(uint8_t seed[3]); 
void kaelRand_lcg24Seed(uint8_t seed[3], const char* cstring);

void kaelRand_hash(uint8_t *numArr, uint8_t arrLen, const char* cstring);