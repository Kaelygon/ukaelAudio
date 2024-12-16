
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/math/math.h"
#include "kaelygon/math/k32.h"
#include "kaelygon/global/kaelMacros.h"


void unit_printBase256Oper(kael32_t *a32, kael32_t *b32, char *a32Cstr, char *b32Cstr, char oper){
   k32_getBase256Cstr(a32Cstr, a32);
   k32_getBase256Cstr(b32Cstr, b32);
   printf("(%s) %c (%s) = ",a32Cstr, oper, b32Cstr);
}

void unit_printBase10Oper(kael32_t *a32, kael32_t *b32, char *a32Cstr, char *b32Cstr, char oper){
   k32_getBase10Cstr(a32Cstr, a32);
   k32_getBase10Cstr(b32Cstr, b32);
   printf("(%s) %c (%s) = ",a32Cstr, oper, b32Cstr);
}



void unit_printBase10Result(kael32_t *result, char *result32Cstr){
   k32_getBase10Cstr(result32Cstr, result);
   printf("(%s)",result32Cstr);
}

void unit_printBase256Result(kael32_t *result, char *result32Cstr){
   k32_getBase256Cstr(result32Cstr, result);
   printf("(%s)",result32Cstr);
}



kael32_t unit_oper32(kael32_t *a32, kael32_t *b32, char oper){
   kael32_t result32={0};

   char result32Cstr[KAEL32_BYTES*4];
   char a32Cstr[KAEL32_BYTES*4];
   char b32Cstr[KAEL32_BYTES*4];

   switch(oper){
      case '*':
         k32_mul(&result32, a32, b32);
         break;
      case '/':
         k32_div(&result32, a32, b32);
         break;
      case '+':
         k32_add(&result32, a32, b32);
         break;
      case '-':
         k32_sub(&result32, a32, b32);
         break;
   }

   printf("b 10: ");
   unit_printBase10Oper(a32, b32, a32Cstr, b32Cstr,oper);
   unit_printBase10Result(&result32, result32Cstr);
   printf("\n");

   printf("b256: ");
   unit_printBase256Oper(a32, b32, a32Cstr, b32Cstr,oper);
   unit_printBase256Result(&result32, result32Cstr);
   printf("\n");


   return result32;
}

int main(){
   kael32_t a32={{0,0,4,4}};
   kael32_t b32={{0,0,4,4}};

   unit_oper32(&a32, &b32, '-');
   printf("\n");

   unit_oper32(&a32, &b32, '+');
   printf("\n");

   kael32_t result32 = unit_oper32(&a32, &b32, '*');
   printf("\n");

   k32_shl(&result32, 4);
   char result32Cstr[KAEL32_BYTES*4];
   unit_printBase256Result(&result32, result32Cstr);
   printf("\n");
   
   unit_oper32(&a32, &b32, '/');
   printf("\n");
}