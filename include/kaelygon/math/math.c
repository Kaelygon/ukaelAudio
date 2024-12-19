/**
 * @file math.c
 * 
 * @brief Header, Strictly unsigned arithmetic
 */
#include "kaelygon/math/math.h"

typedef uint16_t kmath_t;
static const kmath_t SIGN_MASK = 1U<<(sizeof(kmath_t)*CHAR_BIT-1);

#define KMATH_MAX UINT16_MAX

kmath_t kaelMath_min(kmath_t a, kmath_t b){
	return a<b ? a : b;
}

kmath_t kaelMath_max(kmath_t a, kmath_t b){
	return a>b ? a : b;
}

kmath_t kaelMath_isNegative(kmath_t a){
	return a&SIGN_MASK;
}

//Greater than 0 signed
kmath_t kaelMath_gtZeroSigned(kmath_t a){
	return (a!=0) && (!kaelMath_isNegative(a));
}

//ignore sign
kmath_t kaelMath_abs(kmath_t a){
	return a&~SIGN_MASK;
}

//uint subtract that doesn't underflow
kmath_t kaelMath_sub(kmath_t a, kmath_t b){
	return a >= b ? a - b : 0;
}

kmath_t kaelMath_rorr(kmath_t num, kmath_t shift){
	kmath_t invShift=sizeof(kmath_t)*CHAR_BIT;
	invShift=invShift-shift;
	num = (num>>shift) | (num<<invShift);
	return num;
}


uint8_t kaelMath_u8rorr(uint8_t num, uint8_t shift){
	uint8_t invShift=sizeof(uint8_t)*CHAR_BIT;
	invShift=invShift-shift;
	num = (num>>shift) | (num<<invShift);
	return num;
}

//other RLCG parameters
//{shift, mult, add} //Z score - period 65535
//{1, 95, 975}, // 0.316218
//{2, 83, 89},  // 0.374768
//{2, 61, 394}, // 0.513991
//{1, 49, 112}, // 0.645737

/**
 * @brief 16-bit uniformly distributed pseudo random number generator
 */
kmath_t kaelMath_lcg(kmath_t seed){
	seed = kaelMath_rorr(seed,2);
	seed = seed * 83 + 89;
	return seed;
}

uint8_t kaelMath_log10(kmath_t num){
	return 	num<10 ? 0 :
				num<100 ? 1 :
					num<1000 ? 2 : 
						num<10000 ? 3 : 4
	;
}


//Simple cstring to hash 
void kaelRand_hash(char* cstr, uint8_t *numArr, uint8_t arrLen){
	if(NULL_CHECK(cstr) || NULL_CHECK(numArr) || arrLen==0 ){return;}
	const uint8_t ASCII_START = 32;
	const uint8_t ASCII_END = 126;
	const uint8_t ASCII_SCALE = (UINT8_MAX) / (ASCII_END - ASCII_START)+1;

	//shift and scale ascii range 0 to 255. Sum and wrap to mix  
	uint8_t index = 0;
	uint8_t sum = 0;
    while(cstr[index] && (index!=UINT8_MAX)){
		sum += ((uint8_t)(cstr[index])-ASCII_START) * ASCII_SCALE;
		numArr[index%arrLen] = sum;
		index++;
    };
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