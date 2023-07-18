#pragma once

#include <stdint.h>


static uint64_t rdtsc(){
	uint32_t lo,hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi << 32) | lo;
}


static uint16_t rdrand(){
		uint16_t rnum;
		__asm__ __volatile__ ("rdrand %0"  : "=r" (rnum));
    return rnum;
}


//Fast filtering condition
//if( [0 to 2^k] & [num with n 1s] ) 
//when applied to all values 0 to 2^k and mask has n bits, this condition is false 2^k/2^n times 
//if( [0 to 2^16] & 0b1111 ) -> 2^16/2^4 = 4096 times false or 6.25%
//
//an other way but harder to foranumte true/false ratio
//val = [0 to 2^k]
//if(val&(val>>n))

typedef struct {
    uint16_t a;
    uint16_t b;
    uint8_t s;
} ukaelEntropy;

//white noise generator
//lcg with rolling mults and adds
//#define KAENTROPY_LONGER 1 
//KAENTROPY_LONGER is 38% slower but but better dieharder results
//without KAENTROPY_LONGER dieharder result are weaker, but this is still plenty random by ear
ukaelEntropy KAENTROPY = {.a = 13381U, .b = 42533U, .s = 0U};
inline static void ukaelReseed(){
	static const uint8_t mul[] = {13,17,11, 7,23, 3,29,21};
	static const uint8_t add[] = {17,19,23,31,13,27, 3, 7};

		KAENTROPY.a = KAENTROPY.a * mul[KAENTROPY.s] + add[KAENTROPY.s];
		KAENTROPY.b = KAENTROPY.b * mul[KAENTROPY.s] + add[KAENTROPY.s];
		KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

#if KAENTROPY_LONGER==1 
    KAENTROPY.a ^= KAENTROPY.a << 7;
    KAENTROPY.a ^= KAENTROPY.a >> 9;
#endif

	if( KAENTROPY.b&0b0011000101000100U ){return;} //shift mult add array
		KAENTROPY.s++;
		if(KAENTROPY.s==8){KAENTROPY.s=0;}

    if( KAENTROPY.b&(KAENTROPY.a>>8) ){return;} 
		KAENTROPY.s++;
		if(KAENTROPY.s==8){KAENTROPY.s=0;}

    return;
}

//bad testing example for comparison. 
//-42 to -48db variance, ~0.5s loop
inline static void ukaelBadReseed(){

	uint16_t anum,bnum;

	anum=((KAENTROPY.a>>((KAENTROPY.a&7)+3))&255)|1;
	bnum=((KAENTROPY.b>>((KAENTROPY.b&7)+7))&127)|1;

	KAENTROPY.a = KAENTROPY.a * anum + bnum;
	KAENTROPY.b = KAENTROPY.b * anum + bnum;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;
    return;
}

//Seed from rdtsc.
//3.6 times slower than ukaelReseed. Passes all dieharder preset tests
inline static void ukaelRdtscSeed(){
	uint16_t anum,bnum;

	__asm__ __volatile__ ("rdtsc" : "=a" (KAENTROPY.a));
	anum=((KAENTROPY.a>>((KAENTROPY.a&7)+3))&255)|1; //odd 1 to 511
	bnum=((KAENTROPY.b>>((KAENTROPY.b&7)+7))&255)|1;
	
    KAENTROPY.a ^= (KAENTROPY.a) << 7;
    KAENTROPY.a ^= (KAENTROPY.a) >> 9;

	if(bnum>anum){ //better results with bigger multiplier
		uint16_t buf = anum;
		anum=bnum;
		bnum=buf;
	}

	KAENTROPY.a = KAENTROPY.a * anum + bnum;
	KAENTROPY.b = KAENTROPY.b * anum + bnum;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

    return;
}


inline static void ukaelSetSeed(uint16_t seeda, uint16_t seedb){
	KAENTROPY.a = seeda;
	KAENTROPY.b = seedb;
}


inline static uint16_t ukaelRand(){
	ukaelReseed();
	return KAENTROPY.a;
}

inline static uint16_t ukaelRandb(){
	ukaelReseed();
	return KAENTROPY.b;
}

inline static uint32_t u32kaelRand(){
	ukaelReseed();
	return ((uint32_t)KAENTROPY.a<<16)|KAENTROPY.b;
}