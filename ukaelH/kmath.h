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

typedef struct {
    uint16_t a;
    uint16_t b;
} ukaelEntropy;

uint32_t INC[9];//DEBUG
uint32_t II=0;

//White noise generator, even distribution
//Updates uint16_t KAENTROPY.a on call
//KAENTROPY.b is only used for seedin
//Variance at 32768hz u8 : -41db to -39db or ~1.01*Samples
ukaelEntropy KAENTROPY = {.a = 13381U, .b = 42513U};
inline static void reseed(){
	KAENTROPY.a = KAENTROPY.a * 7 + 3;
	KAENTROPY.b = KAENTROPY.b * 7 + 3;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

	if(KAENTROPY.b&0b1111100000000000U){return;}
	//chance continuing ~1000 times every 32768, or ~1000hz

II=0;
INC[II]++;	
II+=II<9?1:0;

	KAENTROPY.a = KAENTROPY.a * 11 + 3;
	KAENTROPY.b = KAENTROPY.b * 11 + 3;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;


	uint16_t mask=(UINT16_MAX<<((KAENTROPY.b&15)))+1; //set mask size
	if( KAENTROPY.a&mask ){return;} //~100hz
		
INC[II]++;	
II+=II<9?1:0;

	while(1){ //Low freq noise <2hz
		KAENTROPY.a = KAENTROPY.a * 7 + 3;
		KAENTROPY.b = KAENTROPY.b * 7 + 3;
		KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;
		mask<<=(KAENTROPY.b&1); //shift mask until 0 or break
		if( KAENTROPY.a&(mask) || mask==0 ){break;} 
	
INC[II]++;	
II+=II<9?1:0;

	}
}


/*
ukaelEntropy KAENTROPY = {.a = 131, .b = 231};
inline static void reseed()
{
	KAENTROPY.a = KAENTROPY.a * 5 + 1;
	KAENTROPY.b = KAENTROPY.b * 7 + 1;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

	if(KAENTROPY.b&0b11000000U){return;}//1000hz
	
	KAENTROPY.a = KAENTROPY.a * 5 + 1;
	KAENTROPY.b = KAENTROPY.b * 7 + 1;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

	uint8_t mask=(UINT8_MAX<<((KAENTROPY.b&7)))>>1; //set mask size
		if( KAENTROPY.a&(mask) ){return;} //70hz

	while(1){
		KAENTROPY.a = KAENTROPY.a * 5 + 1;
		KAENTROPY.b = KAENTROPY.b * 7 + 1;
		KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;
		mask<<=(KAENTROPY.b&1); //the bigger mask, the likelier to exit
		if( KAENTROPY.a&(mask) || mask==0 ){break;} //10hz, <1hz 
	}
}


1 1 
3 1
		KAENTROPY.a = KAENTROPY.a * 3 + 24;
		KAENTROPY.b = KAENTROPY.b * 16 + 8;

*/



/*


typedef struct {
    uint16_t a;
    uint16_t b;
} ukaelEntropy;

//White noise generator
//Updates uint16_t KAENTROPY.a on call
//KAENTROPY.b is only used for seedin
//LSG 8 bits are good white noise
//Variance 39.4db -+ 0.4db
//Spectrum extremes at 32768hz u8 : 1470hz -39.8db, 15900hz -39.db
ukaelEntropy KAENTROPY = {.a = 13381U, .b = 42513U};
inline static void reseed()
{
	KAENTROPY.a = KAENTROPY.a * 5 + 1;
	KAENTROPY.b = KAENTROPY.b * 7 + 1;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

	if(KAENTROPY.b&0b1111100000000000U){return;}//1000hz
	
	KAENTROPY.a = KAENTROPY.a * 5 + 1;
	KAENTROPY.b = KAENTROPY.b * 7 + 1;
	KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;

	uint16_t mask=(UINT16_MAX<<((KAENTROPY.b&15)))>>1; //set mask size
		if( KAENTROPY.a&(mask) ){return;} //70hz

	while(1){
		KAENTROPY.a = KAENTROPY.a * 11 + 33;
		KAENTROPY.b = KAENTROPY.b * 23 + 15;
		KAENTROPY.a = KAENTROPY.a + KAENTROPY.b;
		mask<<=(KAENTROPY.b&1); //the bigger mask, the likelier to exit
		if( KAENTROPY.a&(mask) || mask==0 ){break;} //10hz, <1hz 
	}
}
*/


/*
static const uint8_t sineLut[] = { //if you can spare 256 bytes
	128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,
	176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,
	218,220,222,224,226,228,230,232,234,236,237,239,240,242,243,245,
	246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
	255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,
	246,245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,
	218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,
	176,174,171,168,165,162,159,156,152,149,146,143,140,137,134,131,
	127,124,121,118,115,112,109,106,103, 99, 96, 93, 90, 87, 84, 81,
	 79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
	 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
	  9, 8,  7,  6,  5,  4,  3,  3,  2,  1,  1,  0,  0,  0,  0,   0,
	  0, 0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,   8,
	  9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35,
	 37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76,
	 79, 81, 84, 87, 90, 93, 96, 99,103,107,110,113,116,119,122,125
};
*/