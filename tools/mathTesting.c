
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "kaelygon/math/math.h"

void generateHash(uint8_t *seed, uint8_t *hashArr, uint8_t hashLen, FILE *fptr){
	char * tmpSeed = (char *)seed;
	kaelRand_hash(tmpSeed, hashArr, hashLen);

	//fwrite(hashArr,sizeof(uint8_t),hashLen,fptr); //binary
	fprintf(fptr,"%u ",*((uint16_t*)seed)); //ascii
	fprintf(fptr,"%u\n",*((uint16_t*)hashArr)); 
}

void testHash(FILE *fptr){
	uint32_t testCount = pow(2,16);
	const uint16_t seedSize=4;
	const uint16_t hashSize=2;
	
	uint8_t seed[seedSize];
	uint8_t hash[hashSize];

	memset( seed, (uint8_t)32, seedSize*sizeof(uint8_t) );
	seed[seedSize-1] = '\0';

	for(uint32_t i=0; i!=testCount; i++){

		uint16_t i=0;
		do{
			seed[i]++;
		}while(seed[i]==0 && i++<(seedSize-2));
		if(strlen((char *)seed)<(uint32_t)(seedSize-1)){continue;}

		generateHash(seed, hash, hashSize, fptr);
	}
}

void testRLCG(FILE *fptr){
	uint16_t seed=0;
	for(uint32_t i=0; i<pow(2,16);i++){
		seed=kaelRand_u16lcg(seed);
		fprintf(fptr,"%u %u\n",i,seed);
	}
}

int main(){
	FILE *fptr;
	fptr = fopen("./generated/audio.pcm","wb");
	if(fptr==NULL){printf("open failed\n"); abort();}

//	testRLCG(fptr);
	testHash(fptr);


	fclose(fptr); 
	return 0;
}