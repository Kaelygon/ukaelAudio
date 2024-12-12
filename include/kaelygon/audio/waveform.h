//./include/kaelygon/audio/kaelWaveform.h
//waveform functions
#ifndef KAELWAVEFORM_H
	#define KAELWAVEFORM_H

#include <stdint.h>

#include "audioTypes.h"
#include "tables.h"

/*
	Sine approximation of 127.5*sin(pi*x/2^7)+127.5
	Based on quadratic (3x-x^3)/2 [0,0.25]
	Detailed functions: https://desmos.com/calculator/sqllbjao14
	Max error = ~1.8%. Identical to sine at 0,64,128,192,255
*/
uint8_t kaelAudio_sine(KaelAudio* kaud ){
	uint8_t n = *kaud->wave.curPhase;
	uint8_t q = n>>6; //quarter phase 0b00=1st 0b01=2nd 0b10=3rd 0b11=4th   
	n = n&0b00111111; //repeat quarters
	n = q&0b01 ? 64-n : n; //mirror 2nd and 4th quarters by x-axis
	uint16_t p = (((uint16_t)n*n)>>6)+1; //calculate 6x-n^3/2^11 cube in two parts to prevent overflow //+1 compensates flooring
	p = 6*n - (((uint16_t)n*p)>>5);
	uint8_t o = (p>>1)+kaelAudio_const.silentValue; //scale and offset to match sine wave
	o = q&0b10 ? ~o : o; //mirror 3rd and 4th quarters by y-axis
	return o;
}

uint8_t kaelAudio_saw(KaelAudio* kaud ){
	return *kaud->wave.curPhase+kaelAudio_const.silentValue;
}

uint8_t kaelAudio_square(KaelAudio* kaud ){
	return *kaud->wave.curPhase > (UINT8_MAX>>1) ? UINT8_MAX : 0; //50% duty cycle
}

uint8_t kaelAudio_triangle(KaelAudio* kaud) {
	uint8_t phase = *kaud->wave.curPhase;
	phase+=63;
	uint8_t secondHalf = phase&0b10000000;
	phase <<= 1;
	phase = secondHalf ? ~phase : phase;	//invert 2nd half
	return phase;
}

uint8_t kaelAudio_rorlcg(uint8_t n){
	return ((n>>kaelAudio_const.lcg[2]) | (n<<kaelAudio_const.lcg[3])) * kaelAudio_const.lcg[0] + kaelAudio_const.lcg[1];
}

uint8_t kaelAudio_rand(KaelAudio* kaud){
	uint8_t *curState = &kaud->random.noise[kaud->random.index];
	*curState = kaelAudio_rorlcg(*curState);

	kaud->random.index=(kaud->random.index+1) % kaud->random.size; //increment state
	uint8_t *nextState=&kaud->random.noise[ kaud->random.index];
	*nextState += *curState * kaelAudio_const.lcg[0] + kaelAudio_const.lcg[1]; //mix
	return *curState;
}

uint8_t kaelAudio_noise(KaelAudio* kaud){
	const uint8_t pitch = kaud->wave.info.pitch;
    const uint8_t cond = ( kaud->wave.bufferInc%(pitch+1) == 0 ) ;
	if ( cond ) {
		kaelAudio_rand(kaud);
	}
	return kaud->random.noise[kaud->random.index];
}

//random walk
uint8_t kaelAudio_rwalk(KaelAudio* kaud){
	uint8_t pitch = kaud->wave.info.pitch;
	uint8_t addend = kaelAudio_rand(kaud);
	const uint8_t prevState = kaud->random.rwalk;
	const uint8_t sign = (addend>>3)&0b1; //add=0 subtract=1

	pitch = pitch + addend%2; //alternate to get 0-32 range at 0.5 increments
	addend%=(pitch+1);
	addend= sign ? prevState-addend : prevState+addend; //add or subtract random
	if(sign==0 && addend<prevState){ //overflow
		return UINT8_MAX;
	}else
	if(sign==1 && addend>prevState){ //underflow 
		return 0;
	}
	kaud->random.rwalk=addend;
	return kaud->random.rwalk;
}
#endif