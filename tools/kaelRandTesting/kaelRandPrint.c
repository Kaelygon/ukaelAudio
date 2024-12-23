

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#include <math.h>



#include "./include/kaelRandTesting.h"
//#include "kaelygon/math/rand.h"


int main(){

	PrngCoeff coeff;
	// sanity checks
	//coeff = (PrngCoeff){  0,  0,  0, kaelRandT_gccRand, NULL};
	//coeff = (PrngCoeff){  0,  1,  1, kaelRandT_lcg, NULL}; //add 1

	//Testing
	//4294967296 period, good difference distribution
	coeff = (PrngCoeff) {  8,  1,  205, kaelRandT_simple, "kaelRandT_simple"}; 

   kael32_t randState = {0};

	FILE *fptr;
	fptr = fopen("./generated/audio.pcm","wb");
	if(fptr==NULL){printf("open failed\n"); abort();}

	uint8_t num = 0; 
	uint64_t startTime = __rdtsc();
	for(uint64_t i=0; i<(uint64_t)(pow(2,24));i++){
		num = kaelRandT_base(&randState, coeff);
		fwrite(&num,sizeof(uint8_t),1,fptr); //binary
		//fprintf(fptr,"%u ",num); //ascii
		//printf("%u ",num); //terminal
	}
	uint64_t endTime = __rdtsc();
	printf("time %.4f \n", (double)(endTime-startTime)/3700000000);

	fclose(fptr); 

	printf("num %u \n", num);

	
	return 0;
}
//{  1,  29,  71, kaelRandT_pcg, "kaelRandT_pcg"}, // Z:0.00 - {  0,   0,   0},  - fail:0 Z:0.00, at 16765849 - period 16765849 -         71 98 140 243 221 27 172 186 46 168 18 192 