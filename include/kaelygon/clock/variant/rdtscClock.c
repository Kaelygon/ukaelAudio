/**
 * @file rdtscClock.c
 * 
 * @brief Implementation, clock using rdtsc
 * 
 */

//Include own header
#include "kaelygon/clock/variant/rdtscClock.h"

void kaelClock_rdtsc_sleep(ktime_t sleepTime) {
	ktime_t scaledSleep = sleepTime*CLOCK_TARGET_RATIO;
	usleep(scaledSleep);
}

ktime_t kaelClock_rdtsc_time(){
	ktime_t time = __rdtsc()/CLOCK_TARGET_RATIO;
	return time;
}
