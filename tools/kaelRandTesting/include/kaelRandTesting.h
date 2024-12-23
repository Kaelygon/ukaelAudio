#pragma once
//1-byte array permuted congruential generator
/*
Goals: 
	Speed 
	Limited to 8-bit operations
	Uniform distribution
	Deterministic
	Period is at least 2^(stateCount*8 bits) and up to 2^((sc+1)*8) due to krand.prev
	when constants are well chosen

	Audibly random
		Ideal 0-20khz fequency analysis would be same amplitude for every frequency
		
		I generated white noise, each frequency uniformly distributed -44db
		Then I generated second white noise track but set everything except the band around 3000hz to -100db
		The band amplitude was lowered until it was not audible, amplitude difference is read from plot spectrum 
		https://manual.audacityteam.org/man/plot_spectrum.html

		Bands amplitude by width that are just audible in white noise:
		[band width where mean is 3000hz] - [band amplitude in decibels above the noise]
		<5hz (tones) - 8.0 db
		25hz -	5.6 db
		50hz -	4.4 db
		100hz - 4.0 db
		500hz - 3.2 db
		2000hz - 2.2 db

*/

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "./k32/k32.h"


typedef struct PrngCoeff PrngCoeff;

typedef uint8_t (*PrngOper)(kael32_t *krand, const PrngCoeff coeff);

//PRNG constants
struct PrngCoeff{
    uint8_t shift; 
    uint8_t mul; 
    uint8_t add; 
    PrngOper oper; //Function operatoion applied to the base PRNG
	const char *name; //optional for printing
};


//sanity checking with gcc rand
uint8_t kaelRandT_gccRand( uint8_t num, const PrngCoeff coeff __attribute__((unused))  ){ 
	num = rand();
		return num;
}

uint8_t kaelRandT_rorr(kael32_t *krand, const PrngCoeff coeff){
	k32_rorr(krand, coeff.shift);
	k32_u8mad(krand, krand, coeff.mul, coeff.add);
	return krand->s[0];
}

uint8_t lfsr(uint8_t num) {
	uint8_t out = num==0;
	out^=(num>>5);
	out^=(num>>7);
    return (out&0b1) ^ (num<<1);
}

uint8_t kaelRandT_lcg(kael32_t *krand, const PrngCoeff coeff){
	uint8_t carry = k32_u8mad(krand, krand, coeff.mul, coeff.add);
	return krand->s[0] + carry;
}

uint8_t kaelRandT_pcg(kael32_t *krand, const PrngCoeff coeff){
	uint8_t i=0;
	uint8_t sum=0;
	do{
		kaelMath_u8rorr(krand->s[i], coeff.shift);
		sum+=krand->s[i]++;
	}while(krand->s[i++]==0 && i<(KAEL32_BYTES));
	return sum;
}

uint8_t kaelRandT_base( kael32_t *krand, const PrngCoeff coeff ){
	return coeff.oper(krand, coeff);
}

/**
 * @brief Bijective PRNG
 * 
 * Comparable but not equivalent to 257*seed[i++]+carry
 */
uint8_t kaelRandT_simple( kael32_t *krand, const PrngCoeff coeff ){
	uint16_t carry = coeff.add;
	
	carry+= krand->s[0];
	krand->s[0] = carry;
	
	carry+= krand->s[1] + (carry>>coeff.shift);
	krand->s[1] = carry;
	
	krand->s[2]+= carry + (carry>>coeff.shift);
		
	return krand->s[2];
}

//compare two kael32_t states
//0 means they are equal
uint8_t kaelRandT_cmp(const kael32_t *krand1, const kael32_t *krand2){
	if(krand1==NULL || krand2==NULL){ return -1; }
	return memcmp(&krand1->s,&krand2->s,KAEL32_BYTES);
	return 0;
}











/*

//Rotate right
//No null check since this is expected to be run in thight loops
uint8_t rorr(uint8_t num, uint8_t shift){
	uint8_t invShift=sizeof(uint8_t)*CHAR_BIT;
	invShift=invShift-shift;
	return (num>>shift) | ( num<<invShift );
}


uint8_t lfsr(uint8_t num) {
	uint8_t out = 0;
	out^=(num>>3);
	out^=(num>>4);
	out^=(num>>5);
	out^=(num>>7);
    return (out&0b1) ^ (num<<1);
}



//lcg
uint8_t kaelRandT_lcg( uint8_t num, const PrngCoeff coeff  ){ 
	num = num * coeff.mul + coeff.add;
	return num;
}

//rorr lcg
uint8_t kaelRandT_rorr( uint8_t num, const PrngCoeff coeff  ){ 
	num = rorr(num, coeff.shift);
	num = num * coeff.mul + coeff.add;
	return num;
}

//linear feedback shift register lcg
uint8_t kaelRandT_lfsr( uint8_t num, const PrngCoeff coeff  ){ 
	num = lfsr(num);
	num = num * coeff.mul + coeff.add;
	return num;
}


//operation is done only for some bytes
//identical to ./math/rand.h
uint8_t kaelRandT_baseOg( kael32_t *krand, const PrngCoeff coeff ){ 
	//if(krand==NULL || coeff==NULL){return 0;}
	
	for(uint8_t i=0; i<KAEL32_BYTES;i++){ //Cycle bytes
		uint8_t newValue = krand->s[i];
		newValue = coeff.oper(newValue,coeff); //Apply PRNG operation
		krand->prev+= newValue; //mix with previous sum
		krand->s[i] = newValue; //update state

		//Arbitrary condition to break the cycle. The earlier break, the less random result
		//modulus of consequtive ouputs difference and modulus takes biggest hit in distribution uniformity
		//Bit (mod 2) distribution is still good

		//Some conditions cause reduced period but may work with different constants
		//in a way this acts as a carry where each state byte is a digit
		//if( newValue==0 ){ //only break cycle when 0 (slowest)
		//if( newValue!=0 ){ //always break except when 0 (fastest)
		if( newValue!=0 || newValue&0b11111 ){ // 3/8-1/256 chance carry modifying the next elements
			break;
		}
	}

	return krand->prev;
}
*/