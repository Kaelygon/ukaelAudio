//ukaelAudio program
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <portaudio.h>
#include <time.h>

#include "../ukaelH/ukaelTypedefC.h"
#include "../ukaelH/ukaelWaveC.h"
#include "../ukaelH/ukaelToneC.h"

#define PLAYAUDIO 1

int main( int argc, char *argv[] )  {
	
	ukaelTimeSeed(); 

	AudioData sample[SAMPLE_CHANNELS+1]; //+1 buffer channel 

	WaveArg wargs0 = {
		.time = 	DEFAULT_AMPLITUDE,
		.freq = 	(Frac){55,16},	//55,16 = 440hz at 32768hz
		.u8arg = 	{128,0,0,0},
		.u16arg = 	{127,0,0,0}
	};
	WaveArg wargs1 = {
		.time = 	DEFAULT_AMPLITUDE,
		.freq = 	(Frac){1,64},
		.u8arg = 	{128,0,0,0},
		.u16arg = 	{129,0,0,0}
	}; 
	
	//allocate
	sample[0] = (AudioData){
		.data 		= NULL,
		.size 		= FRAMES_PER_BUFFER,
		.position 	= 0,
		.allocated	= 0
	};
	sample[1] = (AudioData){
		.data 		= NULL,
		.size 		= FRAMES_PER_BUFFER,
		.position 	= 0,
		.allocated  = 0
	};
	sample[2] = (AudioData){//buffer, otherwise valgrind will give error at interleaveChannels
		.data 		= NULL,
		.size 		= 0,
		.position 	= 0,
		.allocated  = 0
	};
	

  	//create pointer to AudioData sample[]
	AudioData* channelArray[SAMPLE_CHANNELS+1];

	AudioData rawAudioData = (AudioData){
		.data 		= NULL,
		.size 		= FRAMES_PER_BUFFER,
		.position 	= 0,
		.allocated  = 0
	};

    sampleAlloc(&sample[0]);
    sampleAlloc(&sample[1]);

	#if PLAYAUDIO==1
	//Port audio
	if(1){

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

		FILE *pFile2;
		pFile2 = fopen("audio2.bin", "w");
		uint32_t inc=0;
		while (inc<32768/FRAMES_PER_BUFFER) { 
			inc++;
			// Generate the wave 
			generateTone(&sample[0], 32, "sine", &wargs0 );
			generateTone(&sample[1], 128, "rwalk", &wargs1 );

//			sampleCopy(&sample[1],&sample[0]);

			// Initialize and allocate audio data for each channel
			for (size_t i = 0; i < SAMPLE_CHANNELS+1; i++) {
				channelArray[i] = &sample[i];  // Store the address of each sample in channelArray
			}
			
			interleaveChannels(&rawAudioData,channelArray);
			
			fwrite(rawAudioData.data, sizeof(uint8_t), rawAudioData.size, pFile2);

			//copy to buffer
   			memcpy(PAudioData, rawAudioData.data, audioBufferBytes);

			//wait, bufferTime - [elapsed time]
			clock_gettime( 1, &timend ); //CLOCK_MONOTONIC
			waitTime.tv_nsec=(bufferTime.tv_nsec - ( timend.tv_nsec - timest.tv_nsec ));
			#define WRITE_ONLY 0
			#if WRITE_ONLY==0
						nanosleep( NULL, &waitTime ); 

						//pass to port audio
						Pa_WriteStream(stream, PAudioData, FRAMES_PER_BUFFER);
			#endif			
			clock_gettime( 1, &timest);
			
		}
		free(PAudioData);
		PAudioData=NULL;
		fclose(pFile2);
		pFile2=NULL;

        // Stop and close the stream
        err = Pa_StopStream(stream);
        if (err != paNoError) {
            printf("Failed to stop stream: %s\n", Pa_GetErrorText(err));
        }
        err = Pa_CloseStream(stream);
        if (err != paNoError) {
            printf("Failed to close stream:%s\n", Pa_GetErrorText(err));
        }
		stream=NULL;

		Pa_Terminate();
	}
	rawAudioData.position = 0;
	#endif

	sampleFree(&rawAudioData);
	for(uint8_t i=0;i<SAMPLE_CHANNELS+1;i++){
		sampleFree(&sample[i]);
	}
	
	return 0;
}
