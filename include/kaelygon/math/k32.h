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
#include "kaelygon/global/kaelMacros.h"

#define KAEL32_BITS 32U
#define KAEL32_BYTES ((uint8_t)((KAEL32_BITS+CHAR_BIT-1)/CHAR_BIT)) //ceil 32/(byte width)

#define KAEL32_BASE10_DIGITS 10U //floor log10(2^32)+1
typedef struct{
	uint8_t s[KAEL32_BYTES]; //Least significant byte last, big endian
}kael32_t;


//--- Arithmetic

// k32 by k32

void k32_set(kael32_t *dest, const kael32_t *src); //dest = src

uint8_t k32_cmp(const kael32_t *a, const kael32_t *b); //memcmp
uint8_t k32_eq(const kael32_t *a,const  kael32_t *b);
uint8_t k32_u8eq(const kael32_t *a, const uint8_t b);

kael32_t k32_mul(kael32_t *result, const kael32_t *base, const kael32_t *mul);
kael32_t k32_mad(kael32_t *result, const kael32_t *base, const kael32_t *mul, const kael32_t *add);

kael32_t k32_div(kael32_t *result, const kael32_t *base, const kael32_t *div);

uint8_t k32_add(kael32_t *result, const kael32_t *base, const kael32_t *add);
uint8_t k32_sub(kael32_t *result, const kael32_t *base, const kael32_t *sub);

// k32 by u8

uint8_t k32_u8mul(kael32_t *result, const kael32_t *base, const uint8_t mul);
uint8_t k32_u8mad(kael32_t *result, const kael32_t *base, const uint8_t mul, const uint8_t add);

uint8_t k32_u8div(kael32_t *result, const kael32_t *base, const uint8_t div);

uint8_t k32_u8add(kael32_t *result, const kael32_t *base, const uint8_t add);
uint8_t k32_u8sub(kael32_t *result, const kael32_t *base, const uint8_t sub);


//--- Bytewise

void k32_rorl(kael32_t *base, uint8_t n); 
void k32_rorr(kael32_t *base, uint8_t n);
void k32_shr(kael32_t *base, uint8_t n);
void k32_shl(kael32_t *base, uint8_t n);


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

