/**
 * @file rdtscClock.c
 * 
 * @brief Implementation, clock using rdtsc
 * 
 */

#include "kaelygon/clock/variant/rdtscClock.h"

// Read date time stamp clock return type
typedef unsigned long long rdtsc_t;

void kaelClock_rdtsc_sleep(ktime_t sleepTime) {
	rdtsc_t scaledSleep = (rdtsc_t)sleepTime*(rdtsc_t)CLOCK_TARGET_RATIO;
	rdtsc_t startCycle = __rdtsc();
	
	do{
		__builtin_ia32_pause();
	}while( __rdtsc() - startCycle < scaledSleep );
}

ktime_t kaelClock_rdtsc_time(){
	ktime_t time = __rdtsc()/(rdtsc_t)CLOCK_TARGET_RATIO;
	return time;
}
