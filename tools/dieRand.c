#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../ukaelH/kmath.h"
#include "../ukaelH/ukaelToneC.h"
#include "../ukaelH/ukaelTypedefC.h"
#include "../ukaelH/ukaelWaveC.h"


#define KAENTROPY_LONGER 0

//benchmark cpu cycles
int main (int argc, char *argv[]){
	
	uint8_t TEST_NO = 1;
//0 rand			FAIL sts_runs sts_serial diehard_craps diehard_squeeze diehard_3dsphere diehard_parking_lot rgb_lagged_sum
//1 u32kaelRand		FAIL sts_runs sts_serial, WEAK diehard_craps diehard_3dsphere
//2 ukaelRdtscSeed	WEAK (sometimes) sts_serial
//3 ukaelBadReseed	FAIL all

	if(argv[1]){ 
		char *argStr = argv[1];
		TEST_NO = atoi(argStr);
	}else{
		return 0;
	}

	ukaelRdtscSeed();
	while(1){

		if(TEST_NO==0){
			uint32_t buf = rand();
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==1){
			uint32_t buf = u32kaelRand();
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==2){
			ukaelRdtscSeed();
			uint32_t buf = (uint32_t)KAENTROPY.a<<16|KAENTROPY.b;
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==3){
			ukaelBadReseed();
			uint32_t buf = (uint32_t)KAENTROPY.a<<16|KAENTROPY.b;
			fwrite(&buf, sizeof(buf), 1, stdout);
		}

	}
	return 0;
}