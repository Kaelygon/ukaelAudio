#pragma once

//waveform functions
#include <stdint.h>

#include "ukaelTypedefC.h"
#include "kmath.h"


//sine approximation.
static inline uint8_t ukaelSine(WaveArg *arg) {	
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;	//n=floor(n)
	uint8_t secondHalf = time8 & 0b10000000;			//s=floor(n%255/128)
	time8 <<= 1;										//2*n%255
	uint16_t buf = ((uint16_t)(time8)<<1) - UINT8_MAX;	//(2*n-255)%65536
	time8 = (uint8_t)((buf * buf) >> 9);				//(floor(n^2%65536)/512)%255
	time8 = secondHalf ? time8 : ~time8;				//n = s ? n : 255-n
	return time8;
}


//Crunchy Sine
static inline uint8_t ukaelCSine(WaveArg *arg) {
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b; //frequency

	uint8_t secondHalf = time8 & 0b10000000;
	time8 = !(time8 & 0b01000000) ? ~time8 : time8;	//invert even quarters
	time8 <<= 2;	//2 period saw
	time8 >>= 4;	//square root. Crunchy precision
	time8*=time8;	//square
	time8 >>= 1;	//divide by 2
	time8 = secondHalf ? time8 : ~time8;	//invert 2nd half
	return time8;
}


//triangle
static inline uint8_t ukaelTriangle(WaveArg *arg) {
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;
	
	time8+=63;
	uint8_t secondHalf = time8&0b10000000;
	time8 <<= 1;
	time8 = secondHalf ? ~time8 : time8;	//invert 2nd half
	return time8;
}

//Saw
static inline uint8_t ukaelSaw(WaveArg *arg) {
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;

	time8+=127;	//add UINT_MAX/2
	return time8;
}


//Square
static inline uint8_t ukaelSquare(WaveArg *arg) {
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;

	time8 = ( time8>>7 ) ? UINT8_MAX : 0; //hi if 0 to 0.5, low if 0.5 to 1.0
	return time8 ;
}


//Pulse square 2 arguments
static inline uint8_t ukaelPulse(WaveArg *arg) {
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;

	time8 = (time8 >= arg->u8arg[0]) ? UINT8_MAX : 0;	//same as square but dutyCycle sets treshold
	return time8 ;
}



//White noise
static inline uint8_t ukaelNoise(WaveArg *arg) {
	reseed();
	return KAENTROPY.a;
}


//Pass filtered triangle noise, slow
//freq{maxD,minD} D=Delta
//Similar freq values generate harmonics of triangle waves
//{255,1} generates high pitch noise
static inline uint8_t ukaelFiltri(WaveArg *arg) {
	reseed();

	uint8_t sign = arg->u8arg[1]; //add=0 subtract=1

	uint8_t prevSample = arg->u8arg[0];
	uint8_t random = KAENTROPY.a;

	//high pass
	if(random < arg->freq.b){ //too small difference
		random = arg->freq.b;
	}else
	//low pass
	if(random > arg->freq.a){ //too high difference
		random = arg->freq.a;
	}

	random= sign ? prevSample-random : prevSample+random; //add or subtract random
	if(sign==0 && random<prevSample){ //overflow
		arg->u8arg[1] = 1; //invert sign
		random = 254;
	}else
	if(sign==1 && random>prevSample){ //underflow 
		arg->u8arg[1] = 0;
		random = 1;
	}
	arg->u8arg[0]=random;
	return random;
}


//random walk, slow
static inline uint8_t ukaelRWalk(WaveArg *arg) {
	reseed();
	
	uint16_t time16 = KAENTROPY.a;

	uint8_t sign = (time16>>5)&1; //add=0 subtract=1

	uint8_t prevSample = arg->u16arg[0];
	uint8_t random = time16;
	random=(uint16_t)random*arg->freq.a/arg->freq.b; //frequency
	random= sign ? prevSample-random : prevSample+random; //add or subtract random
	if(sign==0 && random<prevSample){ //overflow
		return 255;
	}else
	if(sign==1 && random>prevSample){ //underflow 
		return 0;
	}
	arg->u16arg[0]=random;
	return random;
}

static inline uint8_t ukaelTesting(WaveArg *arg) {
    uint16_t time16 = arg->time;
    
    return time16;
}

/*
//rounded square wave
static inline uint8_t ukaelTesting(WaveArg *arg) {
	uint8_t time8 = arg->time * arg->freq.a / arg->freq.b;
	uint8_t secondHalf = time8 & 0b10000000;
	time8 = !(time8 & 0b01000000) ? time8 : ~time8;	//invert even quarters
	time8<<=1;	//triangle wave
	time8=255/(time8+((time8==0)<<2)); //255/time8
	time8 = secondHalf ? time8 : ~time8;	//invert 2nd half
	return time8;
}

//sine look up table
static inline uint8_t ukaelSineL(WaveArg *arg) {
	
	uint8_t time8 = arg->time * arg->freq.a / arg->freq.b;
		time8=sineLut[time8];
	return time8;
}
*/
/*
// 1/sqrt(x) - 16
static inline uint8_t ukaelTesting(WaveArg *arg) {
	uint8_t time8 = arg->time * arg->freq.a / arg->freq.b;
	time8 = (time8==0) ? 255 : ((UINT8_MAX/(time8+8))<<3)+16-(time8>>6);
	return time8;
}
*/
/*
#include <math.h>
//accurate sine very slow
static inline uint8_t ukaelSinef(WaveArg *arg){
	uint8_t time8 = arg->time*arg->freq.a/arg->freq.b;
	return (sin((float)time8/40.7436654)*128+128);
}
*/
/*whacky saw line
static inline uint8_t ukaelNoiseSaw(WaveArg *arg) {
	uint8_t updater = arg->u8arg[1]++;
		
	uint8_t prevSample = arg->u16arg[0];
	uint16_t random = KAENTROPY.a;

	if( updater > (arg->freq.b) ){ //reseed only periodically
		uint8_t sign = random&1;
		
		reseed();
		
		prevSample+= sign ? -random : random;

		arg->u16arg[0]=prevSample;
		arg->u8arg[1]=0;
		return prevSample;
	}

	uint8_t sign = random&1; //add=0 subtract=1

	prevSample+= sign ? -random/(arg->freq.b) : random/(arg->freq.b);
	arg->u16arg[0]=prevSample;
	return prevSample;
}
*/