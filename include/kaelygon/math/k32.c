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



/**
 * @brief kael32_t dest = src
 */
void k32_set(kael32_t *dest, const kael32_t *src){
	KAEL_ASSERT(dest!=NULL && src!=NULL, "Arg is NULL");

	memcpy(dest, src, KAEL32_BYTES);
}


//-- kael32_t by kael32_t functions

/**
 * @return Returns which bytes differ
 * 
 * if equal, return 0
 * elif *a n:th byte is lower, return 255-n
 * elif *b n:th byte is lower, return n
 */
uint8_t k32_cmp(const kael32_t *a,const  kael32_t *b){
	KAEL_ASSERT(a!=NULL && b!=NULL, "Arg is NULL");

	return memcmp(a,b,KAEL32_BYTES);
}

/**
 *  @return 1 if kael32_t are equal
 */
uint8_t k32_eq(const kael32_t *a,const  kael32_t *b){
	KAEL_ASSERT(a!=NULL && b!=NULL, "Arg is NULL");

	return !k32_cmp(a,b);
}

/**
 *  @return 1 if kael32_t are equal
 */
uint8_t k32_u8eq(const kael32_t *a, const uint8_t b){
	KAEL_ASSERT(a!=NULL, "Arg is NULL");

	return k32_eq(a,&(kael32_t){{0,0,0,b}});
}


/**
 * @brief kael32_t add
 * 
 * @return Carry
 */
uint8_t k32_add(kael32_t *result, const kael32_t *base, const kael32_t *add){
	KAEL_ASSERT(result!=NULL && base!=NULL && add!=NULL, "Arg is NULL");

    uint8_t carry = 0;
    for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
        uint8_t sum = base->s[i] + add->s[i] + carry;
        result->s[i] = sum;
		  carry = (sum < base->s[i]) ? 1 : 0; //carry if overflown
    }
    return carry;
}

/**
 * @brief kael32_t subtract
 * 
 * @return if underflown, return 1; else 0
 */
uint8_t k32_sub(kael32_t *result, const kael32_t *base, const kael32_t *sub){
	KAEL_ASSERT(result!=NULL && base!=NULL && sub!=NULL, "Arg is NULL");

    uint8_t borrow = 0;
    for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
        uint8_t diff = base->s[i] - sub->s[i] - borrow;
        result->s[i] = diff;
        borrow = (diff > base->s[i]) ? 1 : 0; //borrow if underflown
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
	
	kael32_t lowPart = {0};
	kael32_t hiPart = {0};

	for(uint8_t i = KAEL32_BYTES - 1; i != UINT8_MAX; --i){
		kael32_t partialProduct = {0};
		uint8_t carry = add->s[i];

		//Multiply the current byte of mul with each byte of base
		for(uint8_t j = KAEL32_BYTES - 1; j != UINT8_MAX; --j){
				uint16_t product = (uint16_t)mul->s[i] * base->s[j] + carry;

				//Determine where this product contributes to partialProduct
				int16_t resultIndex = i + j - (KAEL32_BYTES - 1);
				if(resultIndex >= 0 && resultIndex < KAEL32_BYTES){
					uint8_t oldValue = partialProduct.s[resultIndex];
					partialProduct.s[resultIndex] += product & 0xFF;

					//Handle carry in the addition
					if(partialProduct.s[resultIndex] < oldValue){
						carry = (product >> 8) + 1;
					}else{
						carry = (product >> 8);
					}
				}else{
					carry += (product >> 8);
				}
		}

		//Add the partialProduct to the lowPart or hiPart
		uint8_t lowCarry = k32_add(&lowPart, &lowPart, &partialProduct);
		k32_u8add(&hiPart, &hiPart, lowCarry);
	}

	k32_set(result, &lowPart);
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
 * @brief Compute kael32_t reciprocal
 * TODO:
 */
void _k32_reciprocal(kael32_t *result, const kael32_t *div) {
	KAEL_ASSERT(div!=0, "Division by 0");
	return;
}


/**
 * @brief kael32_t divide 
 * TODO:
 */
kael32_t k32_div(kael32_t *result, const kael32_t *base, const kael32_t *div) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	KAEL_ASSERT(div!=0, "Division by 0");
	kael32_t remainder = {0};

	return remainder;
}





//--- k32 by u8 functions ---

/**
 * @brief kael32_t multiply by unsigned char
 * 
 * @return Most significant carry
 */
uint8_t k32_u8mul(kael32_t *result, const kael32_t *base, const uint8_t mul) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");

	uint8_t carry = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
		uint16_t product = (uint16_t)base->s[i] * mul + carry;
		result->s[i] = product;
		carry = product>>8;
	}
	return carry;
}

/**
 * @brief kael32_t by u8 division
 * 
 * @return remainder
 */
uint8_t k32_u8div(kael32_t *result, const kael32_t *base, const uint8_t div) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL")
	;
	KAEL_ASSERT(div!=0, "Division by 0");
    uint16_t remainder = 0; // Use 16 bits for intermediate calculations
    for (uint8_t i=0; i<KAEL32_BYTES; i++) { // Start from MSB
        uint16_t current = (remainder << 8) | base->s[i];
        result->s[i] = current / div; 
        remainder = current % div;
    }
    return remainder;
}

/**
 * @brief kael32_t add unsigned char
 * 
 * @return Most significant carry
 */
uint8_t k32_u8add(kael32_t *result, const kael32_t *base, const uint8_t addend){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");

	uint8_t carry = addend;
	for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
		uint8_t oldValue = base->s[i];
		uint8_t sum = oldValue + carry;
		result->s[i] = sum;
		if(sum < oldValue){ //if overflown, carry
			carry = 1;
		}else{
			carry = 0;
			break;
		}
	};
	return carry;
}

/**
 * @brief kael32_t subtract unsigned char
 * 
 * @return if underflown, return 1; else 0
 */
uint8_t k32_u8sub(kael32_t *result, const kael32_t *base, const uint8_t sub){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");

	uint8_t borrow = sub;
	for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
		uint8_t diff = base->s[i] - borrow;
		result->s[i] = diff;
		if(diff > base->s[i]){ //if underflown, borrow
			borrow = 1;
		}else{
			borrow = 0;
			break;
		}
	}
	return borrow;
}

/**
 * @brief kael32_t multiply and add by unsigned char
 * 
 * @return Most significant carry
 */
uint8_t k32_u8mad(kael32_t *result, const kael32_t *base, const uint8_t mul, const uint8_t addend) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	
	uint8_t carry = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
		uint16_t product = (uint16_t)base->s[i] * mul + carry + addend;
		result->s[i] = product;
		carry = product>>8;
	}
	return carry;
}





//--- Bytewise functions ---

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
	KAEL_ASSERT(n <= KAEL32_BYTES, "Rotate left amount exceeds width");

	n = kaelMath_sub(KAEL32_BYTES, n);
	k32_rorr(base,n);
}

/**
 * @brief Shift right by n bytes
 */
void k32_shr(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	KAEL_ASSERT(n <= KAEL32_BYTES, "Right shift amount exceeds width");

	uint8_t count = KAEL32_BYTES-n;
	memmove(base->s+n, base->s, count);
}


/**
 * @brief Shift left by n bytes
 */
void k32_shl(kael32_t *base, uint8_t n){
	KAEL_ASSERT(base!=NULL, "Arg is NULL");
	KAEL_ASSERT(n <= KAEL32_BYTES, "Left shift amount exceeds width");

	uint8_t count = KAEL32_BYTES-n;
	memmove(base->s, base->s+n, count);
	memset(base->s+count, 0, n);
}






//--- C string functions ---

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
	}while ( !k32_u8eq(&bufNum,0) );

	cstr[pos] = '\0';
	_k32_reverseCstr(cstr,pos);
}

/**
 * @brief Print base 256 big endian
 * 
 */
void k32_getBase256Cstr(char* cstr, const kael32_t *base){
	if ( NULL_CHECK(cstr) || NULL_CHECK(base) ){return;}

	if( k32_u8eq(base,0) ){ //if 0
		cstr[0]='0';
		cstr[1]='\0';
		return;
	}

	uint8_t pos = 0;
	for(uint8_t i=KAEL32_BYTES-1; i!=UINT8_MAX; --i){
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




//-- Rand functions

//base 256 rand
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