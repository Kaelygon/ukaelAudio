//./include/kaelygon/math/rand.c
//8-bit permuted congruential generator rorr + LCG

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "kaelygon/math/rand.h"

//Simple cstring to hash 
void kaelRand_hash(char* cstr, uint8_t *numArr, uint8_t arrLen){
	if(NULL_CHECK(cstr) || NULL_CHECK(numArr) || arrLen==0 ){return;}
	const uint8_t ASCII_START = 32;
	const uint8_t ASCII_END = 126;
	const uint8_t ASCII_SCALE = (0xFF) / (ASCII_END - ASCII_START)+1;

	//shift and scale ascii range 0 to 0xFF. Sum and wrap to mix  
	uint8_t index = 0;
	uint8_t sum = 0;
    while(cstr[index] && (index!=0xFF)){
		sum += ((uint8_t)(cstr[index])-ASCII_START) * ASCII_SCALE;
        numArr[index%arrLen] = sum;
		index++;
    };
}

//Seed from string hash. NULL string = zero
void kaelRand_seed(KaelRand *krand, char *cstr){
	if(NULL_CHECK(krand)){return;}
	memset(&krand->state,0,sizeof(uint8_t)*KAELRAND_STATES);
	if(cstr==NULL){return;} //Valid use of NULL
	krand->last=(uint8_t)cstr[0]*KAELRAND_MUL+KAELRAND_ADD;
	kaelRand_hash( cstr, krand->state, KAELRAND_STATES );
}

//Iterate KaelRand states and output a pseudo random number
//No null check as this is run in tight loops, CHECK SEPARATELY
uint8_t kaelRand( KaelRand *krand ){ 
	for(uint8_t i=0; i<KAELRAND_STATES;i++){ //Cycle all states condotionally
		krand->state[i] = kaelMath_u8rorr(krand->state[i], KAELRAND_SHIFT) * KAELRAND_MUL + KAELRAND_ADD; //rorr LCG
		krand->last+= krand->state[i]; //mix with previous sum

		//early exit condition. Only breaking the cycle at 0 is necessary for full period.
		//the part after || results in effective 5/8 chance to skip next bytes 
		//trade off for speed at cost off randomness quality
		if( krand->state[i]==0 || ((krand->state[i]&0b11111)!=0) ){ 
			break;
		}
	}
	return krand->last;
}


//16-bit full period RORR LCG
uint16_t kaelRand_u16lcg(uint16_t seed){
	//other 16-bit RLCG parameters 
	//{shift, mul, add}, //ZScore
	//{1, 95, 975}, // 0.316218
	//{2, 83, 89},  // 0.374768
	//{2, 61, 394}, // 0.513991
	//{1, 49, 112}, // 0.645737
	const uint16_t shift= 2;
	const uint16_t mul	= 83;
	const uint16_t add	= 89;
    seed = kaelMath_rorr(seed,shift);
    return seed * mul + add;
}