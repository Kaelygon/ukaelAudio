//./include/kaelygon/clock/clock.c

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <x86intrin.h>

#include "kaelygon/kaelMacros.h" 
#include "kaelygon/clock/clock.h"
#include "kaelygon/math/math.h"

void kaelClock_init(KaelClock *clock){
	if (NULL_CHECK(clock)) { return; }
	//set clock and dispaly update rate
	clock->timeUnit = 32768;
	clock->unitCycles = CLOCK_SPEED_HZ/clock->timeUnit;
	clock->frameStartTime = kaelClock_timeUnits(*clock);

	clock->frameRate = 32;
	clock->unitsPerFrame = (clock->timeUnit)/clock->frameRate;
	clock->frameCounter = 0;
}

/*
	Placeholder x86 clock with 64-bit rdtsc
	Change these to what is available on the current system
*/
ktime_t kaelClock_timeUnits(const KaelClock clock){
	ktime_t time = __rdtsc()/clock.unitCycles;
	return time;
}

void kaelClock_sleepUnits(const KaelClock clock, ktime_t sleepTime) {
	ktime_t placeholderSleepTime=sleepTime*clock.unitCycles/(CLOCK_SPEED_HZ/1000000);
	usleep(placeholderSleepTime);
//	ktime_t start=kaelClock_timeUnits(clock);
//	ktime_t elapsed=0;
//	do{
//		//just that the cpu isn't pinned to 100% during testing
//		ktime_t placeholderSleepTime=(sleepTime-elapsed)*clock.unitCycles/(CLOCK_SPEED_HZ/1000000);
//		usleep(placeholderSleepTime);
//		elapsed=kaelClock_timeUnits(clock)-start;
//	}while(elapsed<sleepTime);
}

ktime_t kaelClock_sync(KaelClock *clock){
	clock->frameCounter++;
	// Sync screen update to CPU clock
	ktime_t timeNow = kaelClock_timeUnits(*clock);
	ktime_t elapsed=0;
	if(timeNow < clock->frameStartTime){ //clock had wrapped around
		ktime_t wrappedTime = KTIME_MAX - clock->frameStartTime; //time between frameStartTime and overflow
		elapsed = wrappedTime + timeNow; //corrected time
	}else{
		elapsed = timeNow - clock->frameStartTime; //delta when no overflow
	}
    ktime_t waitTime = kaelMath_sub(clock->unitsPerFrame, elapsed); //prevent underflow
    clock->frameStartTime = timeNow+waitTime; //New frame starts after the wait
	kaelClock_sleepUnits(*clock, waitTime);
	return 0;
}


ktime_t kaelClock_getFrame(const KaelClock *clock){
	return clock->frameCounter;
}
