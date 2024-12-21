/**
 * @file math.h
 * 
 * @brief Implementation, Strictly unsigned arithmetic
 */
#pragma once

#include <stdint.h>
#include <limits.h>
#include "kaelygon/global/kaelMacros.h"

typedef uint16_t kmath_t;

kmath_t kaelMath_min(kmath_t a, kmath_t b);
kmath_t kaelMath_max(kmath_t a, kmath_t b);
kmath_t kaelMath_isNegative(kmath_t a);
kmath_t kaelMath_gtZeroSigned(kmath_t a);
kmath_t kaelMath_abs(kmath_t a);
kmath_t kaelMath_sub(kmath_t a, kmath_t b);

kmath_t kaelMath_lcg(kmath_t seed);

kmath_t kaelMath_rorr(kmath_t num, kmath_t shift);

uint8_t kaelMath_u8rorr(uint8_t num, uint8_t shift);

uint8_t kaelMath_log10(kmath_t num);


void kaelRand_hash(const char* cstr, uint8_t *numArr, uint8_t arrLen);
uint16_t kaelRand_u16lcg(uint16_t seed);