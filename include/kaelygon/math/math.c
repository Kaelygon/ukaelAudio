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
/**
 * @brief Is the MSB 1
 */
kmath_t kaelMath_isNegative(kmath_t a){
	return a&SIGN_MASK;
}

/**
 * @brief Is the number greater than 0 signed
 */
kmath_t kaelMath_gtZeroSigned(kmath_t a){
	return (a!=0) && (!kaelMath_isNegative(a));
}

/**
 * @brief Ignore sign bit
 */
kmath_t kaelMath_abs(kmath_t a){
	return a&~SIGN_MASK;
}

/**
 * @brief uint subtract that doesn't underflow
 */
kmath_t kaelMath_sub(kmath_t a, kmath_t b){
	return a >= b ? a - b : 0;
}
/**
 * @brief rotate right 16-bit
 */
kmath_t kaelMath_rorr(kmath_t num, kmath_t shift){
	kmath_t invShift=sizeof(kmath_t)*CHAR_BIT;
	invShift=invShift-shift;
	num = (num>>shift) | (num<<invShift);
	return num;
}

/**
 * @brief rotate right 8-bit
 */
uint8_t kaelMath_u8rorr(uint8_t num, uint8_t shift){
	uint8_t invShift=sizeof(uint8_t)*CHAR_BIT;
	invShift=invShift-shift;
	num = (num>>shift) | (num<<invShift);
	return num;
}

/**
 * @brief Same as digitCount - 1
 */
uint8_t kaelMath_log10(kmath_t num){
	return 	num<10 ? 0 :
				num<100 ? 1 :
					num<1000 ? 2 : 
						num<10000 ? 3 : 4
	;
}

/**
 * @brief 16-bit uniformly distributed pseudo random number generator
 */
kmath_t kaelMath_lcg(kmath_t seed){
	seed = kaelMath_rorr(seed,2);
	seed = seed * 83 + 89;
	return seed;
}

/**
 * @brief 16-bit RORR Linear congruent generator constants
 * 
other 16-bit RLCG parameters 
Constants don't follow same convention as regular LCG

I have yet to formulate other method finding these 
other than by brute force
{shift, mul, add}, //ZScore
{1, 95, 975}, // 0.316218
{2, 83, 89},  // 0.374768
{2, 61, 394}, // 0.513991
{1, 49, 112}, // 0.645737
*/

static const uint8_t _LCGShift = 2;
static const uint8_t _LCGMul = 83; 
static const uint8_t _LCGAdd = 89;

/**
 * @brief 16-bit full period RORR LCG
 * 
 */
uint16_t kaelRand_u16lcg(uint16_t seed){
	seed = kaelMath_rorr(seed,_LCGShift);
	return seed * _LCGMul + _LCGAdd;
}

 
/**
 * @brief cstring is hashed into *numArray. 
 * 
 * Near bijective when arrLen = strlen. The exception being cstrings with nulls before end
 * cstr must be NULL terminated. Mixes up to 256 char long strings
 */
void kaelRand_hash(const char* cstring, uint8_t *numArr, uint8_t arrLen){
	if(NULL_CHECK(cstring) || NULL_CHECK(numArr) || arrLen==0 ){
		return;
	}
	uint8_t seedLength = strlen(cstring);
	memset(numArr,0,arrLen);
	uint16_t sum=3083;
	for(uint16_t i=0; i<seedLength; i+=1){
		sum = kaelMath_u8rorr(sum+cstring[i], _LCGShift) * _LCGMul + _LCGAdd; //8-bit RLCG
		numArr[i%arrLen]+= sum;  
	};
}