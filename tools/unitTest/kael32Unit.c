
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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




// TODO:
void unit_32Test(kael32_t *a32, kael32_t *b32){
	printf("------ u32 by u32 Testing ------\n\n");
	kael32_t remainder = unit_oper32(a32, b32, '/');
	char result32Cstr[KAEL32_BYTES*4];
	k32_getBase10Cstr(result32Cstr, &remainder);
	printf("remainder: %s",result32Cstr);
	printf("\n");



	*a32=(kael32_t){{124,1,8,8}};
	*b32=(kael32_t){{0,0,0,4}};
	remainder = unit_oper32(a32, b32, '/');
	k32_getBase10Cstr(result32Cstr, &remainder);
	printf("remainder: %s",result32Cstr);
	printf("\n");



	*a32=(kael32_t){{0,0,7,16}};
	*b32=(kael32_t){{0,0,0,7}};
	remainder = unit_oper32(a32, b32, '/');
	k32_getBase10Cstr(result32Cstr, &remainder);
	printf("remainder: %s",result32Cstr);
	printf("\n");

	printf("\n");
}

void kael32_unit(){

	kael32_t a32={.s={0,0,0,16}};
	kael32_t b32={.s={0,0,0,4}};
	uint8_t b8 = b32.s[KAEL32_BYTES-1]+1;
	

	unit_testOper32u8(&a32, b8);
	unit_testOper32(&a32, &b32);
	unit_32Test(&a32, &b32);


 

	printf("kaelString_unit done\n");
}



int main(){

	kael32_unit();

	return 0;
}