#pragma once

#include <stdint.h>
#include <time.h>


static uint64_t rdtsc(){
	uint32_t lo,hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi << 32) | lo;
}

/*
static uint16_t rdrand(){ //buggy on amd?
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
} ukaelEntropy;

ukaelEntropy UKAEL_STATE = {.a = 13381U, .b = 42533U};

//white noise generator
//1.8 times faster than rand()
//fails ~half of dieharder tests, but no audible patterns
//frequency analysis: <1db variance over 327680 samples
inline static void ukaelReseed(){
	UKAEL_STATE.a = (UKAEL_STATE.a>>11)|(UKAEL_STATE.a<<5); //shift and wrap msb to lsb
	UKAEL_STATE.a^= (UKAEL_STATE.b<<1)+13;
	UKAEL_STATE.b^= (UKAEL_STATE.a<<2)+11;

	return;
}

//Seed from time.h
//1.3 times faster than rand()
//1.4 times slower than ukaelReseed()
//passes most dieharder tests, 3 fails 3 weaks
inline static void ukaelTimeSeed(){
	UKAEL_STATE.a^=time(NULL);
	
	UKAEL_STATE.a = (UKAEL_STATE.a>>11)|(UKAEL_STATE.a<<5); //bit wrap
	UKAEL_STATE.b^= UKAEL_STATE.a*73+31; 

	UKAEL_STATE.b = (UKAEL_STATE.b>>11)|(UKAEL_STATE.b<<5);
	UKAEL_STATE.a^= UKAEL_STATE.b*71+37;
	
	return;
}

inline static void ukaelSetSeed(uint16_t seeda, uint16_t seedb){
	UKAEL_STATE.a = seeda;
	UKAEL_STATE.b = seedb;
}

inline static uint16_t ukaelRand(){
	ukaelReseed();
	return UKAEL_STATE.a;
}

inline static uint16_t ukaelRandb(){
	ukaelReseed();
	return UKAEL_STATE.b;
}


//playground

inline static uint32_t u32kaelRand(){
	ukaelReseed();
	return ((uint32_t)UKAEL_STATE.b<<16)|UKAEL_STATE.a;
}

//bad, testing example for comparison. 
//-42 to -48db variance, 16384 sample loop
inline static void ukaelBadReseed(){
//poor

	uint16_t anum,bnum;

	anum=((UKAEL_STATE.a>>((UKAEL_STATE.a&7)+3))&511)|1;
	bnum=((UKAEL_STATE.b>>((UKAEL_STATE.b&7)+7))&127)|1;

	UKAEL_STATE.a = UKAEL_STATE.a * anum + bnum;
	UKAEL_STATE.b = UKAEL_STATE.b * anum + bnum;
	UKAEL_STATE.a = UKAEL_STATE.a + UKAEL_STATE.b;

	return;
}

ukaelEntropy UKAEL_STA = {.a = 0, .b = 0};

inline static void ukaelSetSeedTesting(){
	UKAEL_STA.a^=time(NULL);
	
	UKAEL_STA.b = (UKAEL_STA.b>>9)|(UKAEL_STA.b<<7);
	UKAEL_STA.a^= UKAEL_STA.b*71+37;
	UKAEL_STA.b^= UKAEL_STA.a*73+31;
}

//decent dieharder tests being this simple
//1.3 times slower than ukaelReseed() due to multiplication
inline static void ukaelReseedTesting(){
	UKAEL_STA.b = (UKAEL_STA.b>>9)|(UKAEL_STA.b<<7);
	UKAEL_STA.a^= UKAEL_STA.b*71+37;
	UKAEL_STA.b^= UKAEL_STA.a*73+31;

	return;
}

inline static uint32_t u32kaelRandTesting(){
	ukaelReseedTesting();
	return ((uint32_t)UKAEL_STA.b<<16)|UKAEL_STA.a;
}