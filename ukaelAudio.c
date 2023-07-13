//ukaelAudio program
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <portaudio.h>
#include <time.h>

#include "ukaelH/ukaelTypedefC.h"
#include "ukaelH/ukaelWaveC.h"
#include "ukaelH/ukaelToneC.h"

#define PLAYAUDIO = 1;

int main( int argc, char *argv[] )  {
	
	ENTROPY^=rdrand(ENTROPY);

	AudioData sample[SAMPLE_CHANNELS+1]; //+1 buffer channel to prevent seg fault 
//55,16 = 440hz at 32768hz
	WaveArg wargs0 = {
		.time = 	DEFAULT_AMPLITUDE,
		.freq = 	(Frac){1,32},
		.u8arg = 	{0,0,0,0},
		.u16arg = 	{127,0,0,0}
	};

	WaveArg wargs1 = {
		.time = 	DEFAULT_AMPLITUDE,
		.freq = 	(Frac){55,16},
		.u8arg = 	{0,0,0,0},
		.u16arg = 	{129,0,0,0}
	};

	//allocate
	sample[0] = (AudioData){
		.data 		= NULL,
		.size 		= BUFFER_SIZE/2,
		.position 	= 0,
		.allocated	= 0
	};
	sample[1] = (AudioData){
		.data 		= NULL,
		.size 		= BUFFER_SIZE/2,
		.position 	= 0,
		.allocated  = 0
	};
    sampleAlloc(&sample[0]);
    sampleAlloc(&sample[1]);

    // Generate the wave
	generateTone(&sample[0], 255, "rwalk", &wargs0 ); 	//write random walk to sample[1] at amplitude 255
	generateTone(&sample[1], 64, "triangle", &wargs1 );

	//TODO: mixing, time tables, midi-like setup, and other functions

  	//create pointer to AudioData sample[]
	AudioData* channelArray[SAMPLE_CHANNELS+1];

	// Initialize and allocate audio data for each channel
	for (size_t i = 0; i < SAMPLE_CHANNELS+1; i++) {
		channelArray[i] = &sample[i];  // Store the address of each sample in channelArray
	}
	
	
	AudioData rawAudioData;
	rawAudioData.allocated=0;
	rawAudioData.size=0;
	rawAudioData.position=0;
	interleaveChannels(&rawAudioData,channelArray);

	//write audio
	audioDataToBin("audio.bin", &rawAudioData);

	#ifdef PLAYAUDIO
	//Port audio
	if(rawAudioData.size!=0){

		PaError err = 0;
        err = Pa_Initialize();
        if (err != paNoError) {
            printf("Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
            return 1;
        }

		// Create an array of stereo samples
		StereoSample* PAudioData = (StereoSample*)malloc(FRAMES_PER_BUFFER * sizeof(StereoSample));

		// Generate or provide audio data in the stereo format

		PaStream* stream;
		err = Pa_OpenDefaultStream(&stream,
							0,                      // No input channels
							PA_CHANNELS,            // 2 output channels for stereo
							paUInt8,              // Sample format
							SAMPLE_RATE,
							FRAMES_PER_BUFFER,
							NULL,                   // No callback, blocking mode
							NULL);                  // No user data
        if (err != paNoError) {
            printf("Failed to open stream: %s\n", Pa_GetErrorText(err));
            Pa_Terminate();
            return 1;
        }
		
        // Start the stream
        err = Pa_StartStream(stream);
        if (err != paNoError) {
            printf("Failed to start stream: %s\n", Pa_GetErrorText(err));
            Pa_CloseStream(stream);
            Pa_Terminate();
            return 1;
        }

		// Play the audio by repeatedly providing the interleaved stereo data

		struct timespec timest, timend, waitTime, bufferTime;
		timest.tv_nsec=0;
		bufferTime.tv_nsec = (1000000000.0*(FRAMES_PER_BUFFER)/SAMPLE_RATE); //wait time in nanoseconds
		uint16_t audioBufferBytes = PA_CHANNELS*FRAMES_PER_BUFFER*sizeof(uint8_t);

		while (rawAudioData.size>=rawAudioData.position+audioBufferBytes) { 

			//TODO: generate audio during buffer wait

			//copy to buffer
   			memcpy(PAudioData, rawAudioData.data+rawAudioData.position, audioBufferBytes);
			rawAudioData.position+=audioBufferBytes;

			//wait, bufferTime - [elapsed time]
			clock_gettime( 1, &timend ); //CLOCK_MONOTONIC
			waitTime.tv_nsec=(bufferTime.tv_nsec - ( timend.tv_nsec - timest.tv_nsec ));
			nanosleep( NULL, &waitTime ); 
			
			//pass to port audio
			Pa_WriteStream(stream, PAudioData, FRAMES_PER_BUFFER);
			clock_gettime( 1, &timest);
			
		}

        // Stop and close the stream
        err = Pa_StopStream(stream);
        if (err != paNoError) {
            printf("Failed to stop stream: %s\n", Pa_GetErrorText(err));
        }
        err = Pa_CloseStream(stream);
        if (err != paNoError) {
            printf("Failed to close stream:%s\n", Pa_GetErrorText(err));
        }

		Pa_Terminate();
		free(PAudioData);

	}
	rawAudioData.position = 0;
	#endif

	sampleFree(&rawAudioData);
	for(uint8_t i=0;i<SAMPLE_CHANNELS;i++){
		sampleFree(&sample[i]);
	}
	
	
return 0;
}
