/**
 * @file clock.c
 * 
 * Clock implementations, Simulate 32768 hz clock
*/

#include "kaelygon/clock/clock.h"

void kaelClock_init(KaelClock *clock){
	if (NULL_CHECK(clock)) { return; }

	clock->frameStartTime = 0;
	clock->frameHigh = 0;
	clock->frameLow = 0;

	clock->frameRate = AUDIO_SAMPLE_RATE/AUDIO_BUFFER_SIZE; 
	clock->cyclesPerFrame = (TARGET_CLOCK_HZ)/clock->frameRate;
}

void kaelClock_sleep(ktime_t cycles){
	#if CLOCK_USING_RDTSC
		kaelClock_rdtsc_sleep(cycles);
	#endif
}

ktime_t kaelClock_time(){
	#if CLOCK_USING_RDTSC
		return kaelClock_rdtsc_time();
	#endif
	return 0;
}

ktime_t kaelClock_sync(KaelClock *clock){
	clock->frameLow++;
	clock->frameHigh += clock->frameLow==0 ;

	// Sync screen update to CPU clock
	ktime_t timeNow = kaelClock_time();
	ktime_t elapsed=0;

	if(timeNow < clock->frameStartTime){ //clock had wrapped around
		ktime_t wrappedTime = KTIME_MAX - clock->frameStartTime; //time between frameStartTime and overflow
		elapsed = wrappedTime + timeNow; //corrected time
	}else{
		elapsed = timeNow - clock->frameStartTime; //delta when no overflow
	}

	uint8_t timerOverrun = elapsed > clock->cyclesPerFrame;
   ktime_t waitTime = timerOverrun ? 0 : clock->cyclesPerFrame - elapsed; //prevent underflow
   clock->frameStartTime = timeNow+waitTime; //New frame starts after the wait
	kaelClock_sleep(waitTime);
	return 0;
}


ktime_t kaelClock_getFrame(const KaelClock *clock){
	return clock->frameLow;
}