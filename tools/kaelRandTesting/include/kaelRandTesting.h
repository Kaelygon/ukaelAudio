
#ifndef TOOLS_KAELPCG_H
#define TOOLS_KAELPCG_H
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

#include <stdint.h>
#include <stdlib.h>

#ifndef CHAR_BIT
	#define CHAR_BIT 8
#endif

typedef uint8_t krand_t;

typedef struct {
	uint8_t *state; //PRNG state
	uint8_t count; //number of elements in state
	uint8_t prev; //each iteration state elements are conditionally summed to this, then returned
}KaelRand;


typedef struct PrngCoeff PrngCoeff;

typedef uint8_t (*PrngOper)(uint8_t num, const PrngCoeff coeff);

//PRNG constants
struct PrngCoeff{
    uint8_t shift; 
    uint8_t mul; 
    uint8_t add; 
    PrngOper oper; //Function operatoion applied to the base PRNG
	const char *name; //optional for printing
};

KaelRand *kaelRandT_new(uint8_t stateCount){
	KaelRand *krand = malloc(sizeof(KaelRand)); 
	if(krand==NULL){return NULL;}

	krand->state = calloc(stateCount, sizeof(uint8_t));
	if(krand->state==NULL){free(krand); return NULL;}

	krand->prev = 0;
	krand->count = stateCount;
	return krand;
}

void kaelRandT_del(KaelRand **krand){
	if(krand==NULL || *krand==NULL){return;}

	free((*krand)->state);
	free(*krand);
	*krand=NULL;
}


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




//sanity checking with gcc rand
uint8_t kaelRandT_gccRand( uint8_t num, const PrngCoeff coeff __attribute__((unused))  ){ 
	num = rand();
		return num;
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
uint8_t kaelRandT_base( KaelRand *krand, const PrngCoeff coeff ){ 
	//if(krand==NULL || coeff==NULL){return 0;}
	
	for(uint8_t i=0; i<krand->count;i++){ //Cycle bytes
		uint8_t newValue = krand->state[i];
		newValue = coeff.oper(newValue,coeff); //Apply PRNG operation
		krand->prev+= newValue; //mix with previous sum
		krand->state[i] = newValue; //update state

		//Arbitrary condition to break the cycle. The earlier break, the less random result
		//modulus of consequtive ouputs difference and modulus takes biggest hit in distribution uniformity
		//Bit (mod 2) distribution is still good

		//Some conditions cause reduced period but may work with different constants
		//in a way this acts as a carry where each state byte is a digit
		//if( newValue==0 ){ //only break cycle when 0 (slowest)
		//if( newValue!=0 ){ //always break except when 0 (fastest)
		if( newValue==0 || newValue&0b11111 ){ // 3/8-1/256 chance carry modifying the next elements
			break;
		}
	}

	return krand->prev;
}







//Returns state array byte count
uint8_t kaelRandT_getStateCount(KaelRand *krand){
	if(krand==NULL){return 0;}
	return krand->count;
}

//*stateArray must be at least as big as krand->state
void kaelRandT_seed(KaelRand *krand, uint8_t *stateArray){
	if(krand==NULL || krand->state==NULL){return;}

	krand->prev = 0;
	if(stateArray==NULL){ //Null = Zero all
		memset(krand->state, 0, krand->count);
		return;
	}
	memcpy(krand->state, stateArray, krand->count);
}

//compare two KaelRand states
//0 means they are equal
uint8_t kaelRandT_cmp(const KaelRand *krand1, const KaelRand *krand2){
	if(krand1==NULL || krand2==NULL){ return -1; }

	if(krand1->count != krand2->count){ return 1; }
	if(krand1->prev != krand2->prev){ return 1; }

	for(uint8_t i=0; i < krand1->count; i++){
		if(krand1->state[i] != krand2->state[i]){
			return 1;
		}
	}
	return 0;
}

#endif





