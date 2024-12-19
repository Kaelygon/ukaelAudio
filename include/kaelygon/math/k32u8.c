/**
 * @file k32u8.c
 * 
 * @brief Implementation file kael32_t k32 by unsigned char functions
 * 
 * Most significant byte is first in array
 */

#include "kaelygon/math/k32u8.h"

//------- k32 by u8 functions ------


kael32_t k32_u8set(const uint8_t b){
	kael32_t u32 = {0};
	u32.s[KAEL32_BYTES-1]=b;
	return u32;
}

/**
 * @brief kael32_t by u8 division
 * 
 * @return remainder
 */
uint8_t k32_u8div(kael32_t *result, const kael32_t *base, const uint8_t div) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
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
 * @brief kael32_t subtract unsigned char
 * 
 * @return if underflown, return 1; else 0
 */
uint8_t k32_u8sub(kael32_t *result, const kael32_t *base, const uint8_t sub){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	k32_set(result,base);
	uint8_t borrow = sub;
	for(uint8_t i=KAEL32_BYTES-1; i!=0xFF; --i){
		uint16_t diff = base->s[i] - borrow;
		result->s[i] = diff;
		if(diff > 0xFF){ //if underflown, borrow
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
	
	uint8_t carry = addend;
	for(uint8_t i=KAEL32_BYTES-1; i!=0xFF; --i){
		uint16_t product = (uint16_t)base->s[i] * mul + carry;
		result->s[i] = product;
		carry = product>>8;
	}
	return carry;
}

/**
 * @brief kael32_t multiply by unsigned char
 * 
 * @return Most significant carry
 */
uint8_t k32_u8mul(kael32_t *result, const kael32_t *base, const uint8_t mul) {
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	uint8_t carry = k32_u8mad(result, base, mul, 0);
	return carry;
}

/**
 * @brief kael32_t add unsigned char
 * 
 * @return Most significant carry
 */
uint8_t k32_u8add(kael32_t *result, const kael32_t *base, const uint8_t addend){
	KAEL_ASSERT(result!=NULL && base!=NULL, "Arg is NULL");
	uint8_t carry = k32_u8mad(result, base, 1, addend);
	return carry;
}

