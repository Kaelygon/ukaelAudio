/**
 * @file math.c
 * 
 * @brief Header, Strictly unsigned arithmetic
 */
#include "kaelygon/math/math.h"

static const uint16_t SIGN_MASK = 1U<<(sizeof(uint16_t)*CHAR_BIT-1);

#define KMATH_MAX UINT16_MAX

inline uint16_t kaelMath_min(uint16_t a, uint16_t b){
	return a<b ? a : b;
}

inline uint16_t kaelMath_max(uint16_t a, uint16_t b){
	return a>b ? a : b;
}
/**
 * @brief Is the MSB 1
 */
inline uint16_t kaelMath_isNegative(uint16_t a){
	return a&SIGN_MASK;
}

/**
 * @brief Is the number greater than 0 signed
 */
inline uint16_t kaelMath_gtZeroSigned(uint16_t a){
	return (a!=0) && (!kaelMath_isNegative(a));
}

/**
 * @brief Ignore sign bit
 */
inline uint16_t kaelMath_abs(uint16_t a){
	return a&~SIGN_MASK;
}

/**
 * @brief uint subtract that doesn't underflow
 */
inline uint16_t kaelMath_sub(uint16_t a, uint16_t b){
	return a >= b ? a - b : 0;
}

/**
 * @brief Same as digitCount - 1
 */
uint8_t kaelMath_log10(uint16_t num){
	return 	num<10 ? 0 :
				num<100 ? 1 :
					num<1000 ? 2 : 
						num<10000 ? 3 : 4
	;
}

/**
 * @brief rotate right 16-bit
 */
inline uint16_t kaelMath_rorr(uint16_t num, const uint16_t shift){
	uint8_t invShift = sizeof(uint16_t)*CHAR_BIT - shift;
	return ((num>>shift) | (num<<invShift));
}

/**
 * @brief rotate right 8-bit
 */
inline uint8_t kaelMath_u8rorr(uint8_t num, const uint8_t shift){
	uint8_t invShift = sizeof(uint8_t)*CHAR_BIT - shift;
	return ((num>>shift) | (num<<invShift));
}




//------ kaelRand ------

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

static const uint8_t _LCGShift = 2U;
static const uint8_t _LCGMul = 83U;
static const uint8_t _LCGAdd = 89U;

/**
 * @brief 16-bit uniformly distributed pseudo random number generator
 */
inline uint16_t kaelRand_lcg(uint16_t seed){
	return kaelMath_rorr(seed, _LCGShift) * _LCGMul + _LCGAdd;
}

/**
 * @brief cstring is hashed into *numArray. 
 * 
 * Near bijective when arrLen = strlen. The exception being cstrings with nulls before end
 * cstr must be NULL terminated. Mixes up to 256 char long strings
 */
void kaelRand_hash(uint8_t *numArr, uint8_t arrLen, const char* cstring){
	if(NULL_CHECK(cstring) || NULL_CHECK(numArr) || arrLen==0 ){
		return;
	}
	uint8_t seedLength = strlen(cstring);
	memset(numArr,0,arrLen);
	uint8_t sum=78; // any number, just an offset
	for(uint8_t i=0; i<seedLength; i+=1){
		sum+= cstring[i]; 
		sum = kaelMath_u8rorr(sum, _LCGShift) * _LCGMul + _LCGAdd; //8-bit RLCG
		numArr[i%arrLen]+= sum;  
	};
}


void kaelRand_lcg24Seed(uint8_t seed[3], const char* cstring){
	kaelRand_hash(seed, 3, cstring);
}

/**
 * @brief Bijective 3-byte PRNG
 * 
 * Comparable but not equivalent to seed[i]*257+carry
 */
uint8_t kaelRand_lcg24(uint8_t seed[3]){
	uint16_t carry = 205;
	
	carry+= seed[0];
	seed[0] = carry;
	
	carry+= seed[1] + (carry>>8);	// These two lines could be put inside a for loop
	seed[1] = carry;					// and iterated through seed[i], i=0 to array length
	
	seed[2]+= carry + (carry>>8);
		
	return seed[2];
}