
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>



#include "kaelygon/math/math.h"

/**
 * @brief Bijective 3-byte PRNG
 * 
 * Comparable but not equivalent to seed[i]*257+carry
 */
uint8_t kaelRand_lcg24_for(uint8_t seed[3]){
	uint16_t carry = 205;

	for(uint8_t i=0;i<3;i++){
		carry+= seed[i] + (carry>>8);
		seed[i] = carry;
	}
		
	return seed[2];
}

int main(){
	FILE *fptr; 
	fptr = fopen("./generated/audio.pcm","wb");
	if(fptr==NULL){printf("open failed\n"); abort();}

	uint8_t seedSize=3;
	uint8_t seed[seedSize];
	memset(seed, 0, sizeof(uint16_t));
	for(uint32_t i=0; i<(uint32_t)pow(2, 16);i++){
		uint16_t num = kaelRand_lcg24(seed);
		fwrite(&num,sizeof(uint8_t),1,fptr); //binary
	}

	fclose(fptr); 
	return 0;
}