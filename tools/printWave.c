#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../ukaelH/ukaelWaveC.h"
#include "../ukaelH/ukaelToneC.h"



#define SAMPLE_COUNT 5000
//benchmark cpu cycles
int main(){
	AudioData sample;
	sample.size=SAMPLE_COUNT;
	sample.allocated=0;
	sampleAlloc(&sample);
KAENTROPY.a=111;
KAENTROPY.b=79;
	
	WaveArg args = {
		.time = 128,
		.freq = (Frac){1,1},
		.u8arg = {128,128,128,128},
		.u16arg = {128,128,128,128}
	};

	generateTone(&sample, 255, "noise", &args );
	
	for(int16_t i=0;i<SAMPLE_COUNT;i+=1){
//		printf("(%d,%d)",i,sample.data[i]);
		printf("%d",sample.data[i]);
		if(i<SAMPLE_COUNT-1){printf(",");}
	}
	printf("\n");

	sampleFree(&sample);

	return 0;
}