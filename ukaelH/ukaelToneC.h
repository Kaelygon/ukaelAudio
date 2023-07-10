#pragma once

//audio generating and playback functions
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <portaudio.h>

#include "ukaelTypedefC.h"
#include "ukaelWaveC.h"

// Callback function that is called by PortAudio to fill the output buffer
static int audioCallback(
	const void* inputBuffer, 
	void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags, 
	void* userData
){
	AudioData* audioData = (AudioData*)userData;
	uint8_t* out = (uint8_t*)outputBuffer;
	uint16_t remainingData = audioData->size - audioData->position;
	uint16_t numFrames = framesPerBuffer;
	
	// If there is not enough audio data remaining, fill the buffer with silence
	if (remainingData < framesPerBuffer) {
		numFrames = remainingData;
		for (uint16_t i = 0; i < numFrames; i++) {
			out[i] = DEFAULT_AMPLITUDE;  // DEFAULT_AMPLITUDE represents silence for unsigned 8-bit audio
		}
	}
	
	// Copy the audio data to the output buffer
	for (uint16_t i = 0; i < numFrames; i++) {
		out[i] = audioData->data[audioData->position++];
	}
	
	return paContinue;
}

//file IO

//Write AudioData to file
static void audioDataToBin(const char* filename, AudioData* rawData) {
	if( !(rawData->size) || rawData->data == NULL ){
		printf("audioDataToBin() : Nothing to write to file.\n");
		rawData->size=0;
		return;
	}

	// Open the file in binary mode for writing
	FILE* file = fopen(filename, "wb");
	if (file == NULL) {
		printf("audioDataToBin() : Failed to open the file for writing.\n");
		return;
	}

	// Write the samples data to the file
	uint16_t bytesWritten = fwrite(rawData->data, sizeof(uint8_t), rawData->size, file);
	if (bytesWritten != rawData->size) {
		printf("audioDataToBin() : Failed to write the rawData to the file.\n");
	}

	// Close the file
	uint8_t closeResult = fclose(file);
	if (closeResult != 0) {
		printf("audioDataToBin() : Failed to close the file.\n");
	}
}




//Tone generation

//List of waveform functions
static const WaveMap waveList[] = {
	//(uint8)
	{"sine", 		ukaelSine		},
	{"saw", 		ukaelSaw		},
	{"noise",	 	ukaelNoise		},
	{"triangle", 	ukaelTriangle	},
	{"square", 		ukaelSquare		},
	{"pulse", 		ukaelPulse		},
	{"rwalk", 		ukaelRWalk		},
	{"wnoise", 		ukaelWNoise		},
	{"csine", 		ukaelCSine		},
//	{"lsine", 		ukaelSineL		},
//	{"sinef", 		ukaelSinef		},
	{"testing", 	ukaelTesting	} 
};


//Number of elements in the mapping table
static const uint16_t waveCount = sizeof(waveList) / sizeof(waveList[0]);

//Generate and store waveform to buffer
static const void generateTone(
	AudioData* samples,
	uint8_t amplitude,
	const char* waveKey,
	WaveArg* wargs
) {
	wargs->freq.b+=wargs->freq.b==0; //prevent freq division by 0
	
	// Find the waveform function pointers based on the string identifier
	ukaelWave waveFunc = NULL;
	for (uint16_t i = 0; i < waveCount; ++i) {
		if (strcmp(waveKey, waveList[i].id) == 0) {
			waveFunc = waveList[i].waveFunc;
			break;
		}
	}
	if(waveFunc==NULL){
		printf("generateTone () : Invalid key!\n");
		return;
	}

	// Generate
	uint16_t value;
	for (uint16_t i = 0; i < (samples->size); ++i) {
		wargs->time = i; //copy time
		
		//pass arguments and generate sample
		value = waveFunc(wargs); 

		value =  ((value * amplitude)>>8) ;
		value += ((UINT8_MAX - amplitude)>>1); // Amplitude
		samples->data[i] = value; //copy to pointer
	}

	return;
}



//Memory functions

//Allocate and set to DEFAULT_AMPLITUDE. Requires size
static void sampleAlloc(AudioData* destin) {

    //Malloc or alloc
	if(destin->allocated == 1){
		destin->data = realloc(destin->data, destin->size * sizeof(uint8_t));
	}else{
		destin->data = (uint8_t*)malloc(destin->size * sizeof(uint8_t));
		destin->allocated = 1;
	}

	if (destin->data == NULL) {
		printf("sampleCopy() : Failed realloc destin.data\n");
		return;
	}

	// memset to DEFAULT_AMPLITUDE
	memset((void*)destin->data, DEFAULT_AMPLITUDE, destin->size);
	if (destin->data == NULL) {
		printf("sampleAlloc() : Failed set sampleData to silent.\n");
		return;
	}
}

//Free AudioData .data
static void sampleFree(AudioData* destin) {
	if(destin->size == 0 || destin->data==NULL){
		return;
	}
    free((void*)destin->data);
}

//mixing

//Copy and allocate AudioData to destin from source
static void sampleCopy(AudioData* destin, const AudioData* source) {
	
    //Malloc or alloc
    if (destin->size != source->size) {
		if(destin->allocated == 1){
       		destin->data = realloc(destin->data, source->size * sizeof(uint8_t));
		}else{
			destin->data = (uint8_t*)malloc(source->size * sizeof(uint8_t));
			destin->allocated = 1;
		}

        if (destin->data == NULL) {
            printf("sampleCopy() : Failed realloc destin.data\n");
            return;
        }
        destin->size = source->size;
    }
    memcpy((void*)destin->data, (void*)source->data, source->size);
	if (destin->data == NULL) {
		printf("sampleCopy() : Failed to copy samples.\n");
		return;
	}
}

//Append AudioData to desting from source
static void sampleAppend(AudioData* destin, const AudioData* source){
	uint16_t originalSize = destin->size;

	if(destin->allocated == 1){
		destin->data = realloc(destin->data, (originalSize + source->size) * sizeof(uint8_t)); //rescale destin
	}else{
		destin->data = (uint8_t*)malloc( (originalSize + source->size) * sizeof(uint8_t) );
		destin->allocated = 1;
	}

	
	if (destin->data == NULL) {
		printf("sampleAppend() : Failed realloc destin.data\n");
		return;
	}

	memcpy( destin->data + originalSize, source->data, source->size * sizeof(uint8_t) ); //append

	//check if data is null and that last appended value matches 
	uint16_t srcLastIndex = source->size-1;
	uint16_t newLastIndex = originalSize+source->size-1;

	printf("%u,%u\n", destin->data[newLastIndex] , source->data[srcLastIndex]);
	if ( destin->data == NULL || destin->data[newLastIndex] != source->data[srcLastIndex]) {
		printf("sampleAppend() : Failed append to destin.data\n");
		destin->data = realloc( destin->data, originalSize ); //revert scaling
		return;
	}
	destin->size = destin->size+source->size; //set new size
}

//interleaves channel array into output
void interleaveChannels(AudioData* destin, AudioData** channel) {
	uint16_t numChannels = 0;
	uint16_t maxSize=0;
	uint16_t combinedSize=0;

	while( channel[numChannels] != NULL && channel[numChannels]->allocated == 1 ){ //Null checks
		maxSize = (maxSize < (channel[numChannels]->size)) ? channel[numChannels]->size : maxSize; //set maxSize to biggest
		combinedSize+=channel[numChannels]->size; //add up sizes
		numChannels++;
	}
	if(numChannels==0 || numChannels >= 255){
		printf("interleaveChannels() : No channels to interleave.\n");
		return;
	}
	if(combinedSize==0){
		printf("interleaveChannels() : No channels interleaved. Zero size.\n");
		return;
	}

    // Reallocate output data if necessary
    if (destin->size != combinedSize) {
		if(destin->allocated == 1){
       		destin->data = realloc(destin->data, combinedSize * sizeof(uint8_t));
		}else{
			destin->data = (uint8_t*)malloc(combinedSize * sizeof(uint8_t));
			destin->allocated = 1;
		}
        if (destin->data == NULL) {
            printf("interleaveChannels() : Failed realloc destin.data\n");
            return;
        }
        destin->size = combinedSize;
    }

    // Interleave the channel data
    for (uint16_t i = 0; i < maxSize; i++) {
        for (uint16_t j = 0; j < numChannels; j++) {
			if(channel[j]->size >= i){ //Check if increment exceeds current channel size
           		destin->data[i * numChannels + j] = channel[j]->data[i];
			}else{
				destin->data[i * numChannels + j] = DEFAULT_AMPLITUDE;
			}
        }
    }
}
