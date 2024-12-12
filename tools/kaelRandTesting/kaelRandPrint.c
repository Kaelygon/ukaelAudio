

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#include <math.h>



#include "./include/kaelRandTesting.h"


int main(){

	PrngCoeff coeff;
	// sanity checks
	//coeff = (PrngCoeff){  0,  0,  0, kaelRandT_gccRand, NULL};
	//coeff = (PrngCoeff){  0,  1,  1, kaelRandT_lcg, NULL}; //add 1

	// ~2^17 period with 2 stateCount
	//coeff = (PrngCoeff){  0,  37,  57, kaelRandT_lcg, NULL}; 		//Zavg:0.47 //2.9db deviation //FAIL_DIFF Z:5.84  mod 17

	// >2^23.9 period with 2 stateCount
	//coeff = (PrngCoeff){  0,   83, 124, kaelRandT_lfsr, NULL}; 	//Zavg:0.28 //4.0db deviation //FAIL_DIFF Z:9.75  mod 17
	coeff = (PrngCoeff){  3,  83,  63, kaelRandT_rorr, NULL}; 		//Zavg:1.63 //4.4db deviation //FAIL_DIFF Z:13.45 mod 17

	//coeff = (PrngCoeff){  1,   7,  55, kaelRandT_rorr, "kaelRandT_rorr"};


	uint8_t byteCount = 2; 
    KaelRand *randState = kaelRandT_new(byteCount);
	if(randState==NULL){return 0;}

	FILE *fptr;
	fptr = fopen("./generated/audio.pcm","wb");
	if(fptr==NULL){printf("open failed\n"); abort();}

	uint8_t num = 0; 
	uint64_t startTime = __rdtsc();
	for(uint64_t i=0; i<(uint64_t)(pow(2,24));i++){
		num = kaelRandT_base(randState, coeff);
		//fwrite(&num,sizeof(uint8_t),1,fptr);
		fprintf(fptr,"%u ",num);
		//printf("%u ",num);
	}
	uint64_t endTime = __rdtsc();
	printf("time %.4f \n", (double)(endTime-startTime)/3700000000);

	fclose(fptr); 

	printf("num %u \n", num);

	kaelRandT_del(&randState);
	
	return 0;
}
