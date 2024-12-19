/**
 * @file clockTypes.h
 * 
 * @brief Shared headers and definitions for all the different clock variants
 * 
 * Otherwise you'd need to forward declare every clock header file
*/

#pragma once

#include <stdint.h>



//------ CLOCK SYNC DEFINES ------

//Audio will be 32768hz so that will be the default clock
#ifndef AUDIO_SAMPLE_RATE
	#define AUDIO_SAMPLE_RATE 32768U
#endif

#ifndef CLOCK_SPEED_HZ
	#define CLOCK_SPEED_HZ AUDIO_SAMPLE_RATE
#endif
#define TARGET_CLOCK_HZ AUDIO_SAMPLE_RATE

// How many times faster actual clock is than the target clock. 1:1 = no scaling
#define CLOCK_TARGET_RATIO (CLOCK_SPEED_HZ / TARGET_CLOCK_HZ) 



//------ RDTSC ------
 
//RDTSC will be the fallback as afaik it's the most readibly available 
//low lever timer that doesn't require root
#ifndef CLOCK_USING_RDTSC
	#if CLOCK_TARGET_RATIO != 1
		//If clock has to be scaled
		#define CLOCK_USING_RDTSC 1
	#endif
#endif



//------ RTC ------

#ifdef __unix__ 
	//You may need to add permission to read rtc, sudo setfacl -m u:campus:r /dev/rtc
	#define CLOCK_USING_RTC
#else
	//TBD if it's even viable to port this mess to windows
#endif



//------ SHARED TYPES ------

// Since APIC Timer requires root we use __rdtsc
#if __SIZEOF_POINTER__>=8
	typedef uint64_t ktime_t;
	#define KTIME_MAX UINT64_MAX
#elif __SIZEOF_POINTER__>=4
	typedef uint32_t ktime_t;
	#define KTIME_MAX UINT32_MAX
#else
	typedef uint16_t ktime_t;
	#define KTIME_MAX UINT16_MAX
#endif

typedef struct{
	ktime_t frameStartTime; // time (units) when last frame was rendered
	uint16_t frameHigh;
	uint16_t frameLow;
	uint16_t frameRate; // text display refresh rate (Hz)
	ktime_t cyclesPerFrame; // how many units betweeen updates
}KaelClock;