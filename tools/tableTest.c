/**
 * @file bookUnit.c
 * 
 * @brief Test program for book.c
 * 
 * TODO: Pixel mode is currently broken as I am drafting KRLE standard
 * TODO: Create similar functions like KREL->Pixels in krleTGA.c and declare them in krleBase.c 
 * TODO: All colors aren't displayed correctly, The old pixel draw method has collisions with new KRLE standard
 */

#include <stdlib.h>
#include <stdio.h>

#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

void unit_tableTest(){
	for(uint32_t i=0; i<32; i++){
		uint16_t arg2 = krle_colorTable[i &0b11111];
		Krle_ansiStyle ansiStyle = krle_decodeStyle(i);
		printf("%u\n",arg2);
		printf("col %u, back %u, bright %u, style %u\n",ansiStyle.color, ansiStyle.back, ansiStyle.bright, ansiStyle.style);
	}
}

int main() {
	kaelDebug_alloc("kael_bookUnit");

	unit_tableTest();

	kaelDebug_free();

	return 0;
}


