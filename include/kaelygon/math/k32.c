/**
 * @file k32.c
 * 
 * @brief Emulating 32-bit unsigned integer using 4 unsigned char
 * 
 * Most significant byte is first in array
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/math/math.h"
#include "kaelygon/math/k32.h"
#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/k32u8.h"


//------ kael32_t by kael32_t functions ------

/**
 * @brief kael32_t dest = src
 */
void k32_set(kael32_t *dest, const kael32_t *src){
	KAEL_ASSERT(dest!=NULL && src!=NULL, "Arg is NULL");

	memcpy(dest, src, KAEL32_BYTES);
}



/**
 * @return Returns which nth bytes differs
 * 
 * if equal, return 128
 * elif *a n:th byte is lower, return (<128)
 * elif *b n:th byte is lower, return (>128) 
 */
uint8_t k32_cmp(const kael32_t *a,const  kael32_t *b){
	KAEL_ASSERT(a!=NULL && b!=NULL, "Arg is NULL");
	for(uint8_t i=0; i<KAEL32_BYTES; i++){
		if( a->s[i] == b->s[i] ){
			continue;
		}
		return a->s[i] > b->s[i] ? 128U+1+i : 128U-1-i; 
	}
	return 128;
}


/**
 * @brief kael32_t add
 * 
 * @return Carry
 */
uint8_t k32_add(kael32_t *result, const kael32_t *base, const kael32_t *add){
	KAEL_ASSERT(result!=NULL && base!=NULL && add!=NULL, "Arg is NULL");

	uint16_t carry = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=0xFF; --i){
		carry+= base->s[i] + add->s[i];
		result->s[i] = carry;
		carry = (carry>>8); //carry if overflown
	}
	return carry;
}


/**
 * @brief kael32_t subtract old
 * 
* @return if underflown, return 1; else 0
*/
uint8_t k32_sub(kael32_t *result, const kael32_t *base, const kael32_t *sub){
	KAEL_ASSERT(result!=NULL && base!=NULL && sub!=NULL, "Arg is NULL");

	uint16_t borrow = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=0xFF; --i){
		borrow = base->s[i] - sub->s[i] - borrow;
		result->s[i] = borrow;
		borrow = (borrow > base->s[i]) ? 1 : 0; //borrow if underflown
	}
	return borrow;
}

/**
 * @brief kael32_t multiply add
 * 
 * @return Overflown part
 */
kael32_t k32_mad(kael32_t *result, const kael32_t *base, const kael32_t *mul, const kael32_t *add){
	KAEL_ASSERT(result!=NULL && base!=NULL && mul!=NULL && add!=NULL, "Arg is NULL");

	kael32_t hiPart = {0};
	*result = k32_u8set(0); //result is used as the low part
	
	for(uint8_t i = KAEL32_BYTES - 1; i != 0xFF; --i){
		kael32_t partialProduct = {0};
		uint16_t carry = add->s[i];
		uint16_t multiplier = mul->s[i];
		if(multiplier==0 && carry==0){continue;}

		//Multiply the current byte of mul with each byte of base
		for(uint8_t j = KAEL32_BYTES - 1; j != 0xFF; --j){
				uint16_t product = multiplier * base->s[j] + carry;
				if(product==0){continue;}
				
				carry = (product >> 8); //next digit carry

				//Choose partialProduct digit
				uint8_t resultIndex = i + j;
				if(resultIndex<2*KAEL32_BYTES){
					partialProduct.s[resultIndex-(KAEL32_BYTES-1)]+= product;
				}else 
				if(resultIndex<KAEL32_BYTES){
					partialProduct.s[resultIndex]+= product;
				}
		}
		//Add the partialProduct to the lowPart or hiPart
		uint8_t lowCarry = k32_add(result, result, &partialProduct);
		if(lowCarry || carry){
			k32_u8add(&hiPart, &hiPart, lowCarry+carry);
		}
	}

	return hiPart; 
}

/**
 * @brief kael32_t multiply
 * 
 * @return Overflown part
 */
kael32_t k32_mul(kael32_t *result, const kael32_t *base, const kael32_t *mul){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	return k32_mad(result, base, mul, &(kael32_t){0});
}

/**
 * @brief Count leading zero bytes
 * 
 */
uint8_t k32_clz(kael32_t *base){
	for(uint8_t i=0; i<KAEL32_BYTES;i++){
		if(base->s[i]!=0){
			return i;
		}
	}
	return KAEL32_BYTES;
}

/**
 * @brief kael32_t Shift bitwise right 
 * 
 */
void k32_shrBit(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");

	uint8_t carry=0;
	for(uint8_t i=0; i<KAEL32_BYTES; i++){
		uint8_t newCarry = base->s[i] << (CHAR_BIT - n);
		base->s[i] >>= n;
		base->s[i] |= carry;
		carry = newCarry;
	}
}

/**
 * @brief kael32_t Shift bitwise left 
 * 
 */
void k32_shlBit(kael32_t *base, uint8_t n){
    KAEL_ASSERT(base != NULL, "Arg is NULL");

    uint8_t carry = 0;
    for (uint8_t i = KAEL32_BYTES-1; i!=0xFF; --i){
        uint8_t newCarry = base->s[i] >> (CHAR_BIT - n);
        base->s[i] <<= n;
        base->s[i] |= carry;
        carry = newCarry;
    }
}


/**
 * @brief kael32_t division by binary search 
 * 
 * 
 */
kael32_t k32_div(kael32_t *result, const kael32_t *base, const kael32_t *div){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	KAEL_ASSERT( k32_cmp(div,&(kael32_t){0})!=128 , "Division by 0");

	kael32_t guessProduct;
	kael32_t lower={0};
	kael32_t upper;
	k32_set(&upper,base); //base is the upper bound; div by 1

	*result=k32_u8set(0); //Reuse result as X in X*div=base
	uint8_t cmp; //compare result*div = base; cmp>128 bigger-, cmp<128 smaller than X

	while( k32_cmp(&lower, &upper)<=128 ){  
		k32_add(result, &lower, &upper); //result = lower + upper. result is the division guess
		k32_shrBit(result,1); //divide by 2	by 1-bit shift right 

		kael32_t hiPart = k32_mul(&guessProduct, result, div); // Get current product and compare to base

		if( k32_cmp(&hiPart, &(kael32_t){0})>128 ){
			cmp=129; //was overflown
		}else{
			cmp = k32_cmp(&guessProduct, base); 
		}

		// if product is bigger than should, reduce resul
		if(cmp>128){ // result is bigger than X
			k32_u8sub(&upper, result, 1);
		}else if(cmp<128){ // result is smaller than X
			k32_u8add(&lower, result, 1);
		}else{
			return (kael32_t){0}; //remainder 0
		}
	}
		
	if (cmp > 128) { // result was bigger than base; correct value
		k32_u8sub(result, result, 1);
		k32_mul(&guessProduct, result, div);
	}

	kael32_t remainder={0};
	k32_sub(&remainder, base, &guessProduct); 

	return remainder;
}



//------- Bytewise functions ------

/**
 * @brief Rotate right by n bytes
 */
void k32_rorr(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	KAEL_ASSERT(n <= KAEL32_BYTES, "Rotate right amount exceeds width");

	for (uint8_t i = 0; i < KAEL32_BYTES-1; i++) {
		uint8_t newIndex = (i+n)%KAEL32_BYTES;
		uint8_t buf = base->s[i];
		base->s[i] = base->s[newIndex];
		base->s[newIndex] = buf;
	}
}

/**
 * @brief Rotate left by n bytes
 */
void k32_rorl(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	n = kaelMath_sub(KAEL32_BYTES, n);
	k32_rorr(base,n);
}

/**
 * @brief Shift right by n bytes
 */
void k32_shr(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	for (uint8_t i = KAEL32_BYTES-1; i!=0xFF; --i) {
		uint8_t index = i-n;
		base->s[i] = index<KAEL32_BYTES ? base->s[index] : 0;
	}
}


/**
 * @brief Shift left by n bytes
 */
void k32_shl(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	for (uint8_t i = 0; i<KAEL32_BYTES; i++) {
		uint8_t index = i+n;
		base->s[i] = index<KAEL32_BYTES ? base->s[index] : 0;
	}
}




//------- C string functions ------

/**
 * @brief Reverse C string
 */
void _k32_reverseCstr(char* cstr, const uint8_t len){
	if( NULL_CHECK(cstr) ){return;}

	for(size_t i=0; i<len/2; i++){
		uint8_t opposite = len-i-1;
		char bufChar=cstr[i];
		cstr[i]=cstr[opposite];
		cstr[opposite]=bufChar;
	}
}

/**
 * @brief Print kael32_t in base 10
 */
void k32_getBase10Cstr(char* cstr, const kael32_t *base) {
	if( NULL_CHECK(cstr) || NULL_CHECK(base) ){return;}

	kael32_t bufNum;
	k32_set(&bufNum, base);
	uint8_t pos = 0;

	do{
		uint8_t remainder = k32_u8div(&bufNum, &bufNum, 10);
		cstr[pos++] = '0' + remainder;
	}while ( k32_cmp(&bufNum,&(kael32_t){0})!=128 );

	cstr[pos] = '\0';
	_k32_reverseCstr(cstr,pos);
}

/**
 * @brief Print base 256 big endian
 * 
 */
void k32_getBase256Cstr(char* cstr, const kael32_t *base){
	if ( NULL_CHECK(cstr) || NULL_CHECK(base) ){return;}

	if( k32_cmp(base,&(kael32_t){0})==128 ){ //if 0
		cstr[0]='0';
		cstr[1]='\0';
		return;
	}

	uint8_t pos = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=0xFF; --i){
		uint8_t b256Digit = base->s[i];
		if(i!=KAEL32_BYTES-1){
			cstr[pos++]= ' '; //base256 separator
		}
		do{
			uint8_t b10Digit = b256Digit%10;
			b256Digit/=10;
			cstr[pos++]= '0' + b10Digit;
		}while(b256Digit!=0);
	}

	cstr[pos] = '\0';
	_k32_reverseCstr(cstr,pos);
}




//------ Rand functions ------

/**
 * @brief base 256 rand
 * 
 * @warning No NULL_CHECK
 */
uint8_t k32_rand(kael32_t *seed){
	KAEL_ASSERT(seed!=NULL, "Arg in NULL");
	uint8_t carry = k32_u8mad(seed, seed, KAELRAND_MUL, KAELRAND_ADD);
	return seed->s[3] + carry;
}

//seed from string hash. NULL string = zero
void k32_seed(kael32_t *base, char *cstr){
	if(NULL_CHECK(base)){return;}
	memset(&base->s,0,sizeof(uint8_t)*KAEL32_BYTES);
	if(cstr==NULL){return;} //Valid use of NULL
	kaelRand_hash( cstr, base->s, KAEL32_BYTES );
}

#if __SIZEOF_POINTER__>=4

	uint32_t k32_toUint32(const kael32_t *base) {
		uint32_t u32=0;
		for(uint8_t i=0; i<KAEL32_BYTES; i++){
			uint8_t shift = (KAEL32_BYTES*8-(i+1)*8);
			u32|=(uint32_t)base->s[i]<<shift;
		} 
		return u32;
	}

#endif