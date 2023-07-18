#pragma once

#include <stdint.h>


static uint64_t rdtsc(){
	uint32_t lo,hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi << 32) | lo;
}

/*
static uint16_t rdrand(){
		uint16_t rnum;
		__asm__ __volatile__ ("rdrand %0"  : "=r" (rnum));
	return rnum;
}
*/

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

ukaelEntropy UKAEL_LCG = {.a = 13381U, .b = 42533U, .s = 0U};
static const uint8_t UKAEL_MUL[] = {13,17,23, 7,13,19, 3,11};
static const uint8_t UKAEL_ADD[] = {17,19,31,11,23,27,29,21};

//white noise generator
//lcg with rolling mults and adds
//1.8 times faster than rand()
inline static void ukaelReseed(){

		UKAEL_LCG.a = UKAEL_LCG.a * UKAEL_MUL[UKAEL_LCG.s] + UKAEL_ADD[UKAEL_LCG.s];
		UKAEL_LCG.b = UKAEL_LCG.b * UKAEL_MUL[UKAEL_LCG.s] + UKAEL_ADD[UKAEL_LCG.s];
		UKAEL_LCG.a = UKAEL_LCG.a + UKAEL_LCG.b;

	if( UKAEL_LCG.a&0b0011000101010000U ){return;} //random comparison to pass some values
		UKAEL_LCG.s++; //shift mult, add arrays
		if(UKAEL_LCG.s==8){UKAEL_LCG.s=0;}

	if( UKAEL_LCG.b&(UKAEL_LCG.a>>8) ){return;} 
		UKAEL_LCG.s++;
		if(UKAEL_LCG.s==8){UKAEL_LCG.s=0;}

	return;
}

#include <time.h>
//Seed from rdtsc.
//1.5 times slower than rand()
//2.7 times slower than ukaelReseed
inline static void ukaelRdtscSeed(){

	uint16_t buf=time(NULL);
	//finer but twice as slow as time(null), more performant than clock()
	__asm__ __volatile__ ("rdtsc" : "=a" (buf));

	uint8_t shift = buf&15;

	UKAEL_LCG.a = (UKAEL_LCG.a>>(shift))|(UKAEL_LCG.a<<(16-shift));
	UKAEL_LCG.b ^= UKAEL_LCG.a;
		
	UKAEL_LCG.s++;
	if(UKAEL_LCG.s==8){UKAEL_LCG.s=0;}

	UKAEL_LCG.a = UKAEL_LCG.a * UKAEL_MUL[UKAEL_LCG.s] + UKAEL_ADD[UKAEL_LCG.s];
	UKAEL_LCG.b = UKAEL_LCG.b * UKAEL_MUL[UKAEL_LCG.s] + UKAEL_ADD[UKAEL_LCG.s];
	UKAEL_LCG.a = UKAEL_LCG.a + UKAEL_LCG.b;

	return;
}

//bad testing example for comparison. 
//-42 to -48db variance, ~0.5s loop
inline static void ukaelBadReseed(){
//poor

	uint16_t anum,bnum;

	anum=((UKAEL_LCG.a>>((UKAEL_LCG.a&7)+3))&511)|1;
	bnum=((UKAEL_LCG.b>>((UKAEL_LCG.b&7)+7))&127)|1;

	UKAEL_LCG.a = UKAEL_LCG.a * anum + bnum;
	UKAEL_LCG.b = UKAEL_LCG.b * anum + bnum;
	UKAEL_LCG.a = UKAEL_LCG.a + UKAEL_LCG.b;


/*
//real bad
	UKAEL_LCG.a = UKAEL_LCG.a * 7 + 3;
	UKAEL_LCG.b = UKAEL_LCG.b * 7 + 3;
	UKAEL_LCG.a = UKAEL_LCG.a + UKAEL_LCG.b;
*/
/*
//linear
	UKAEL_LCG.a++;
*/

	return;
}


inline static void ukaelSetSeed(uint16_t seeda, uint16_t seedb){
	UKAEL_LCG.a = seeda;
	UKAEL_LCG.b = seedb;
}


inline static uint16_t ukaelRand(){
	ukaelReseed();
	return UKAEL_LCG.a;
}

inline static uint16_t ukaelRandb(){
	ukaelReseed();
	return UKAEL_LCG.b;
}

inline static uint32_t u32kaelRand(){
	ukaelReseed();
	return ((uint32_t)UKAEL_LCG.b<<16)|UKAEL_LCG.a;
}