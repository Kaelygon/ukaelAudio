/**
 * @file clockTypes.h
 * 
 * @brief Shared headers and definitions for all the different clock variants
 * 
 * Otherwise you'd need to forward declare every clock header file
*/

#pragma once

#include <stdint.h>
#include "kaelygon/global/kaelMacros.h"



//------ CLOCK SYNC DEFINES ------


#define CLOCK_SPEED_HZ 3704954300U //AMD Ryzen 5900x 3.7GHz

//Audio will be 32768hz so that will be the default clock
#ifndef AUDIO_SAMPLE_RATE
	#define AUDIO_SAMPLE_RATE 32768U
#endif

#ifndef CLOCK_SPEED_HZ
	#define CLOCK_SPEED_HZ AUDIO_SAMPLE_RATE
#endif
#define TARGET_CLOCK_HZ AUDIO_SAMPLE_RATE

// How many times faster actual clock is than the target clock. 1:1 = no scaling
#define CLOCK_TARGET_RATIO ((CLOCK_SPEED_HZ+TARGET_CLOCK_HZ/2) / TARGET_CLOCK_HZ) //integer round



//------ RDTSC ------
 
//RDTSC will be the fallback as afaik it's the most readibly available 
//low lever timer that doesn't require root
#ifndef CLOCK_USING_RDTSC
	#if CLOCK_TARGET_RATIO != 1
		//If clock has to be scaled
		#define CLOCK_USING_RDTSC 1
	#endif
#endif



//------ SHARED TYPES ------

typedef uint16_t ktime_t;
#define KTIME_MAX UINT16_MAX

typedef struct{
	ktime_t bufferStartTick; // Tick count at beginning of current cycle
	uint16_t tickHigh; // Long time
	uint16_t tickLow; // Short time
	uint16_t tickRate; // Ticks in second. Buffer fill rate per second
	ktime_t ticksPerBuffer; // Ticks between buffer
}KaelClock;