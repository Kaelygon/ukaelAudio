/**
 * @file k32.h
 * 
 * @brief Emulating 32-bit unsigned integer using 4 unsigned char
 * 
 * Most significant byte is first in array
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#define KAEL32_BITS 32U
#define KAEL32_BYTES ((uint8_t)((KAEL32_BITS+CHAR_BIT-1)/CHAR_BIT)) //ceil 32/(byte width)

#define KAEL32_BASE10_DIGITS 10U //floor log10(2^32)+1
typedef struct{
	uint8_t s[KAEL32_BYTES]; //Least significant byte last, big endian
}kael32_t;

#include "./k32u8.h"

//--- Arithmetic

//--- k32 by k32

void k32_set(kael32_t *dest, const kael32_t *src);

uint8_t k32_cmp(const kael32_t *a, const kael32_t *b); // greater>128, equal==128, less<128
uint8_t k32_cmpByte(const kael32_t *a,const  kael32_t *b);

kael32_t k32_mul(kael32_t *result, const kael32_t *base, const kael32_t *mul);
kael32_t k32_mad(kael32_t *result, const kael32_t *base, const kael32_t *mul, const kael32_t *add);

kael32_t k32_div(kael32_t *result, const kael32_t *base, const kael32_t *div);

uint8_t k32_add(kael32_t *result, const kael32_t *base, const kael32_t *add);
uint8_t k32_sub(kael32_t *result, const kael32_t *base, const kael32_t *sub);


//--- Bytewise

void k32_rorl(kael32_t *base, uint8_t n); 
void k32_rorr(kael32_t *base, uint8_t n);
void k32_shr(kael32_t *base, uint8_t n);
void k32_shl(kael32_t *base, uint8_t n);


//--- Bitwise
void k32_shlBit(kael32_t *base, uint8_t n);
void k32_shrBit(kael32_t *base, uint8_t n);


//--- C string

void k32_getBase10Cstr(char* cstr, const kael32_t *base);
void k32_getBase256Cstr(char* cstr, const kael32_t *base);


//--- PRNG

enum KaelRand_const{
	KAELRAND_MUL   		= 89,
	KAELRAND_ADD   		= 57
};

uint8_t k32_rand(kael32_t *krand);
void k32_seed(kael32_t *krand, char *cstr);


// System exclusives that are above 16-bit 
#if __SIZEOF_POINTER__>=4
	uint32_t k32_toUint32(const kael32_t *base);
#endif