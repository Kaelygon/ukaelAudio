#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "../ukaelH/ukaelWaveC.h"
#include "../ukaelH/ukaelToneC.h"


//#define CYCLES_PER_SECOND (3698778634.42102954886)
#define CYCLES_PER_SECOND (3700000000) //3.7ghz
#define SAMPLE_COUNT (1<<15)
#define REPEAT (1<<13)
//benchmark cpu cycles
int main(){
	AudioData sample;
	sample.size=SAMPLE_COUNT;
	sample.allocated=0;

	double avg=0;
	
	WaveArg wargs = {
		.time = 	DEFAULT_AMPLITUDE,
		.freq = 	(Frac){55,16},	//55,16 = 440hz at 32768hz
		.u8arg = 	{128,0,0,0},
		.u16arg = 	{128,0,0,0}
	};
	
	uint64_t cyclesDelta,cyclesst;
	uint8_t warmups=1;

	#define TEST_ALL 1
	for(uint32_t k=0;k<waveCount+warmups;k++){
		sampleAlloc(&sample);
		#if TEST_ALL
			const char* testKey = waveList[k%waveCount].id; //test all functions
		#else
			const char* testKey = "sine"; //test single function
		#endif

		// Generate the sine wave data
		cyclesst = rdtsc();
				for(uint32_t j=0;j<REPEAT;j++){
						generateTone(&sample, 255, testKey, &wargs );
				}
		cyclesDelta = rdtsc()-cyclesst;

		uint16_t r0 = UKAEL_STATE.a;
		uint16_t r1 = UKAEL_STATE.b;

		printf("%s\n",(const char*)testKey);
		printf("Cycles %lu\n",cyclesDelta);
		printf("Clock time %.4f\n",(double)cyclesDelta/CYCLES_PER_SECOND);
		printf("%.1lf M Samples per s\n",(double)(SAMPLE_COUNT*REPEAT)/((double)cyclesDelta/CYCLES_PER_SECOND)/1000000);
		printf("entropy %d,%d\n",r0,r1);
		printf("\n");

		if(k>warmups-1){//if warm up, don't add average
			avg+=(double)(SAMPLE_COUNT*REPEAT)/((double)cyclesDelta/CYCLES_PER_SECOND)/1000000.0f;
		}

		sampleAlloc(&sample); //clear to prevent caching
	}

	
	printf("Avg %.11lf\n",avg/(waveCount));
	sampleFree(&sample);
	
	return 0;
}