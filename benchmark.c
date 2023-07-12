#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "ukaelH/ukaelWaveC.h"
#include "ukaelH/ukaelToneC.h"



//#define CYCLES_PER_SECOND (3698778634.42102954886)
#define CYCLES_PER_SECOND (3700000000) //3.7ghz
#define SAMPLE_COUNT (1<<14)
#define REPEAT (1<<12)
//benchmark cpu cycles
int main(){
	AudioData sample;
	sample.size=SAMPLE_COUNT;
	sample.allocated=0;

	double avg=0;
	
	WaveArg wargs = {
		.time = 128,
		.freq = (Frac){1,3},
		.u8arg = {128,128},
		.u16arg = {128,128}
	};
	
	uint64_t cyclesDelta,cyclesst;
	double timeDelta;
	clock_t timest;
	uint8_t warmups=4;

	#define TEST_ALL 0
	for(uint32_t k=0;k<waveCount+warmups;k++){
		sampleAlloc(&sample);
		#if TEST_ALL
			const char* testKey = waveList[k%waveCount].id; //test all functions
		#else
			const char* testKey = "rwalk"; //test single function
		#endif

		// Generate the sine wave data
		cyclesst = rdtsc();
		timest = clock();
				for(uint32_t j=0;j<REPEAT;j++){
						generateTone(&sample, 255, testKey, &wargs );
				}
		cyclesDelta = rdtsc()-cyclesst;
		timeDelta = (clock()-timest)/1000000.0f;

		printf("%s\n",(const char*)testKey);
		printf("Cycles %lu\n",cyclesDelta);
		printf("Clock time %.4f\n",(double)cyclesDelta/CYCLES_PER_SECOND);
		printf("Systm Time %.4f\n",timeDelta);
		printf("%.1lf M Samples per s\n",(double)(SAMPLE_COUNT*REPEAT)/(timeDelta)/1000000);
		printf("\n");

		if(k>warmups-1){//if warm up, don't add average
			avg+=(double)(SAMPLE_COUNT*REPEAT)/(timeDelta)/1000000.0f;
		}

		sample.data=memset(sample.data,128,sample.size*sizeof(uint8_t)); //clear to prevent caching
		ENTROPY=rand();
	}

	
	printf("Avg %.11lf\n",avg/(waveCount));
	sampleFree(&sample);
	
	return 0;
}