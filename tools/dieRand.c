#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "../ukaelH/kmath.h"
#include "../ukaelH/ukaelToneC.h"
#include "../ukaelH/ukaelTypedefC.h"
#include "../ukaelH/ukaelWaveC.h"


//benchmark cpu cycles
int main (int argc, char *argv[]){

	//0 rand
	//1 u32kaelRand
	//2 ukaelRdtscSeed
	//3 ukaelBadReseed	
	//4 /dev/urandom
	//5 u32kaelRandTesting
	volatile uint8_t TEST_NO = 0;

	if(argv[1]){ 
		char *argStr = argv[1];
		TEST_NO = atoi(argStr);
	}else{
		return 0;
	}

	FILE *fp;
	
	fp = fopen("/dev/urandom", "r");
		
	srand(time(NULL));
	ukaelTimeSeed();
	while(1){

		if(TEST_NO==0){
			uint32_t buf = rand();
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==1){
			uint32_t buf = u32kaelRand();
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==2){
			ukaelTimeSeed();
			uint32_t buf = (uint32_t)UKAEL_STATE.b<<16|UKAEL_STATE.a;
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==3){
			ukaelBadReseed();
			uint32_t buf = (uint32_t)UKAEL_STATE.b<<16|UKAEL_STATE.a;
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==4){
			uint32_t buf;
			fread((char*)(&buf),sizeof(buf),1,fp);
			fwrite(&buf, sizeof(buf), 1, stdout);
		}else if(TEST_NO==5){
			uint32_t buf = u32kaelRandTesting();
			fwrite(&buf, sizeof(buf), 1, stdout);
		}

	}
	

	fclose(fp);
		

	return 0;
}