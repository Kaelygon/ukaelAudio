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
//an other way but harder to formulate true/false ratio
//val = [0 to 2^k]
//if(val&(val>>n))

uint32_t INC[9];//DEBUG
uint32_t II=0;

typedef struct {
    uint16_t a;
    uint16_t b;
    uint8_t s;
} ukaelEntropy;


//white noise generator
//lcg with rolling mults and adds
#define KAENTROPY_LONGER 0 //overkill for waveforms
ukaelEntropy KAENTROPY = {.a = 13381U, .b = 42513U, .s = 0U};
inline static void reseed(){
	static const uint8_t mul[] = {13,17,11, 7,23, 3,29,21};
	static const uint8_t add[] = {17,19,23,31,13,27, 3, 7};

		KAENTROPY.a = KAENTROPY.a * mul[KAENTROPY.s] + add[KAENTROPY.s];
		KAENTROPY.b = KAENTROPY.b * mul[KAENTROPY.s] + add[KAENTROPY.s];
		KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

#if KAENTROPY_LONGER==1 
    KAENTROPY.a ^= KAENTROPY.a << 7;
    KAENTROPY.a ^= KAENTROPY.a >> 9;
#endif

	if( KAENTROPY.b&0b0010001011100000U ){return;} //shift mult add array

		KAENTROPY.s++;
		if(KAENTROPY.s==8){KAENTROPY.s=0;}

    if( KAENTROPY.b&(KAENTROPY.a>>8) ){return;} 
		KAENTROPY.s++;
		if(KAENTROPY.s==8){KAENTROPY.s=0;}

    return;
}
