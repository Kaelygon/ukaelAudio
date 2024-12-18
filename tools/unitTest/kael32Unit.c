
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "kaelygon/math/math.h"
#include "kaelygon/math/k32.h"
#include "kaelygon/global/kaelMacros.h"


void unit_printBase256Oper(const kael32_t *a32, const kael32_t *b32, char *a32Cstr, char *b32Cstr, char oper){
	k32_getBase256Cstr(a32Cstr, a32);
	k32_getBase256Cstr(b32Cstr, b32);
	printf("(%s) %c (%s) = ",a32Cstr, oper, b32Cstr);
}

void unit_printBase10Oper( const kael32_t *a32, const kael32_t *b32, char *a32Cstr, char *b32Cstr, char oper){
	k32_getBase10Cstr(a32Cstr, a32);
	k32_getBase10Cstr(b32Cstr, b32);
	printf("(%s) %c (%s) = ",a32Cstr, oper, b32Cstr);
}



void unit_printBase10Result(const kael32_t *result, char *result32Cstr){
	k32_getBase10Cstr(result32Cstr, result);
	printf("(%s)",result32Cstr);
}

void unit_printBase256Result(const kael32_t *result, char *result32Cstr){
	k32_getBase256Cstr(result32Cstr, result);
	printf("(%s)",result32Cstr);
}


kael32_t unit_oper32( const kael32_t *a32, const kael32_t *b32, char oper){
	kael32_t result32={0};

	char result32Cstr[KAEL32_BYTES*4];
	char a32Cstr[KAEL32_BYTES*4];
	char b32Cstr[KAEL32_BYTES*4];

	switch(oper){
		case '*':
			k32_mul(&result32, a32, b32);
			printf("mul32\n");
			break;
		case '/':
			k32_div(&result32, a32, b32);
			printf("div32\n");
			break;
		case '+':
			printf("add32\n");
			k32_add(&result32, a32, b32);
			break;
		case '-':
			printf("sub32\n");
			k32_sub(&result32, a32, b32);
			break;
	}

	printf("b 10: ");
	unit_printBase10Oper(a32, b32, a32Cstr, b32Cstr, oper);
	unit_printBase10Result(&result32, result32Cstr);
	printf("\n");

	printf("b256: ");
	unit_printBase256Oper(a32, b32, a32Cstr, b32Cstr, oper);
	unit_printBase256Result(&result32, result32Cstr);
	printf("\n");


	return result32;
}


kael32_t unit_oper32u8(const kael32_t *a32, const uint8_t b8, char oper){
	kael32_t result32={0};

	char result32Cstr[KAEL32_BYTES*4];
	char a32Cstr[KAEL32_BYTES*4];
	char b8Cstr[KAEL32_BYTES*4];

	switch(oper){
		case '*':
			printf("mul32u8\n");
			k32_u8mul(&result32, a32, b8);
			break;
		case '/':
			printf("div32u8\n");
			k32_u8div(&result32, a32, b8);
			break;
		case '+':
			printf("add32u8\n");
			k32_u8add(&result32, a32, b8);
			break;
		case '-':
			k32_u8sub(&result32, a32, b8);
			printf("sub32u8\n");
			break;
	}

	kael32_t tmp = k32_u8set(b8);
	printf("b 10: ");
	unit_printBase10Oper(a32, &tmp, a32Cstr, b8Cstr, oper);
	unit_printBase10Result(&result32, result32Cstr);
	printf("\n");

	printf("b256: ");
	unit_printBase256Oper(a32, &tmp, a32Cstr, b8Cstr, oper);
	unit_printBase256Result(&result32, result32Cstr);
	printf("\n");


	return result32;
}


void unit_testOper32u8(kael32_t *a32, const uint8_t b8){
	printf("------ u32 by u8 operations ------\n\n");
	unit_oper32u8(a32, b8, '-');
	printf("\n");

	unit_oper32u8(a32, b8, '+');
	printf("\n");

	unit_oper32u8(a32, b8, '*');
	printf("\n");
	
	unit_oper32u8(a32, b8, '/');
	printf("\n");
}

void unit_testOper32(kael32_t *a32, const kael32_t *b32){
	printf("------ u32 by u32 operations ------\n\n");

	unit_oper32(a32, b32, '-');
	printf("\n");

	unit_oper32(a32, b32, '+');
	printf("\n");

	unit_oper32(a32, b32, '*');
	printf("\n");

//	unit_oper32(a32, b32, '/');
//	printf("\n");
}








void unit_32DivTesting(){
	kael32_t kael32a={.s={0,0,1,255}};
	kael32_t kael32b={.s={0,0,1,3}};

	for(uint32_t i=0; i<20000; i++){
		k32_u8mad(&kael32a,&kael32a,85,85);
		k32_u8mad(&kael32a,&kael32a,13,11);

		uint32_t u32a = k32_toUint32(&kael32a);
		uint32_t u32b = k32_toUint32(&kael32b);

		uint32_t expectedRes32 = u32a/u32b;
		uint32_t expectedRemainder = u32a%u32b;
		
		kael32_t kaelRes32;
		kael32_t remainder = k32_div(&kaelRes32, &kael32a, &kael32b);
		uint32_t convertedRes32 = k32_toUint32(&kaelRes32);
		uint32_t convRemainder = k32_toUint32(&remainder);

		if(expectedRemainder!=convRemainder){
			printf("Expected remainder: %u \n", expectedRemainder);
			printf("Actual remainder: %u \n", convRemainder);
		}

		if(expectedRes32 != convertedRes32){
			printf("Expected product: %u / %u = %u\n",u32a, u32b, expectedRes32);
			printf("Actual product: = %u\n",convertedRes32);
		}

	}

	return;
}


void kael32_mulTesting(){
	kael32_t kael32a={.s={0,1,1,0}};
	kael32_t kael32b={.s={0,1,1,0}};

	for(uint32_t i=0; i<10; i++){
		k32_u8mad(&kael32a,&kael32a,1,4);
		k32_u8mad(&kael32b,&kael32b,1,3);

		uint32_t u32a = k32_toUint32(&kael32a);
		uint32_t u32b = k32_toUint32(&kael32b);

		uint64_t expectedRes64 = (uint64_t)u32a*u32b;
		uint32_t expectedRes32 = expectedRes64&0xFFFFFFFF;
		uint64_t expectedHiPart32 = (uint64_t)expectedRes64>>32U;
		
		printf("\n---- Base conversion ---\n");
		unit_oper32(&kael32a, &kael32b, '*');
		printf("------------------------\n");

		kael32_t kaelRes32;
		kael32_t hiPart = k32_mul(&kaelRes32, &kael32a, &kael32b);
		uint32_t convertedRes32 = k32_toUint32(&kaelRes32);
		uint32_t u32hiPart = k32_toUint32(&hiPart);


		if(expectedRes32 != convertedRes32){
			printf("\n\n---- Discrepency ----\n");
			printf("expected mult: %u * %u = %u\n",u32a, u32b, expectedRes32);
			printf("k32 mult: %u\n",convertedRes32);
			printf("\n");
			printf("expected hiPart: %u\n", (uint32_t)expectedHiPart32);
			printf("k32 hiPart: %u\n\n", u32hiPart);
		}

	}

	return;
}



void kael32_unit(){

	kael32_t a32={.s={0,0,255,255}};
	kael32_t b32={.s={0,0,0,255}};
	uint8_t b8 = b32.s[KAEL32_BYTES-1];
	

	unit_testOper32u8(&a32, b8);
	unit_testOper32(&a32, &b32);
	unit_32DivTesting();
	kael32_mulTesting();

	printf("kaelString_unit done\n");
}

int main(){

	kael32_unit();

	return 0;
}

