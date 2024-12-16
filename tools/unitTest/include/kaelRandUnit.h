/**
 * @file kaelRandUnit.h
 * 
 * @brief Test main math/rand.h multi state PCG
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x86intrin.h>

#include "kaelygon/math/math.h"
#include "kaelygon/math/k32.h"

#include <math.h>

void kaelRand_unit(){

	kael32_t randState; 
	k32_seed(&randState,"I am about to become a hash, tihihii!");

	FILE *fptr;
	fptr = fopen("./generated/audio.pcm","wb");
	if(fptr==NULL){printf("open failed\n"); abort();}

	uint8_t num = 0; 

	uint64_t startTime = __rdtsc();
	for(uint64_t i=0; i<(uint64_t)pow(2,8);i++){
		num = k32_rand(&randState);
		fwrite(&num,sizeof(uint8_t),1,fptr);
	}
	uint64_t endTime = __rdtsc();
	printf("time %.4f \n", (double)(endTime-startTime)/3700000000);

	fclose(fptr); 

	printf("num %u \n", num);

	printf("kaelRand_unit Done\n");	
}
	 