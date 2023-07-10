#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//constants

//portAudio
static const uint16_t SAMPLE_RATE = 32768;
static const uint16_t FRAMES_PER_BUFFER = 64; 
static const uint16_t BUFFER_SIZE = UINT16_MAX;
static const uint8_t PA_CHANNELS = 2;

//ukael
//silence
static const uint8_t DEFAULT_AMPLITUDE = 128;
//sample channels
static const uint8_t SAMPLE_CHANNELS = 2;


//typedef

//AudioData (data*, size, position)
typedef struct {
	uint8_t* data;  // Pointer to the raw audio data
	uint16_t size;     	 // AudioData sample count
	uint16_t position;      // Current position in the audio data
	uint8_t allocated; //this is NULL if not allocated
} AudioData;


//fraction (a, b)
typedef struct {
	uint8_t a;
	uint8_t b;
} Frac;


//wave arguments (uint16_t time, frac, u8arg, u16arg)
typedef struct{
	uint16_t 	time;			//uint16_t time
//	uint32_t 	time;			//won't work with intel 8086
	Frac 		freq;			//Frac{frac.a,frac.b}
	uint8_t 	u8arg[4];		//uint8_t u8arg[4]
	uint16_t 	u16arg[4];		//uint16_t u16arg[4]
} WaveArg;


//wave function wrapper
typedef uint8_t (*ukaelWave)(WaveArg *arg);

// WaveMap (const char* id, WaveArg args)
typedef struct {
	const char* id;
	ukaelWave waveFunc;
} WaveMap;

typedef struct {
    float left;
    float right;
} StereoSample;