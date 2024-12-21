/**
 * @file clock.c
 * 
 * Clock implementations, Simulate 32768 hz clock
*/

#include "kaelygon/clock/clock.h"

/**
 * @brief Fine time clock runs at TARGET_CLOCK_HZ
 * 
 * Rolls over every UINT16_MAX / TARGET_CLOCK_HZ second
 */
ktime_t kaelClock_time(){
	#if CLOCK_USING_RDTSC
		return kaelClock_rdtsc_time();
	#endif
	return 0;
}

void kaelClock_init(KaelClock *clock){
	if (NULL_CHECK(clock)) { return; }

	clock->bufferStartTick = kaelClock_time();
	clock->tickHigh = 0;
	clock->tickLow = 0;

	clock->tickRate = AUDIO_SAMPLE_RATE/AUDIO_BUFFER_SIZE;
	clock->ticksPerBuffer = (TARGET_CLOCK_HZ)/clock->tickRate;
}

void kaelClock_sleep(ktime_t cycles){
	#if CLOCK_USING_RDTSC
		kaelClock_rdtsc_sleep(cycles);
	#endif
}

/**
 * @brief Increment tick and sleep remaining time
 * 
 * Call last in loop
 */
ktime_t kaelClock_sync(KaelClock *clock){
	clock->tickLow++;
	clock->tickHigh += clock->tickLow==0 ;

	ktime_t timeNow = kaelClock_time();
	ktime_t elapsed=0;

	if(timeNow < clock->bufferStartTick){ //clock had wrapped around
		ktime_t wrappedTime = KTIME_MAX - clock->bufferStartTick; //time between bufferStartTick and overflow
		elapsed = wrappedTime + timeNow; //corrected time
	}else{
		elapsed = timeNow - clock->bufferStartTick; //delta when no overflow
	}

	uint8_t timerOverrun = elapsed > clock->ticksPerBuffer;
   ktime_t waitTime = timerOverrun ? 0 : clock->ticksPerBuffer - elapsed; //prevent underflow
   clock->bufferStartTick = timeNow+waitTime; //New tick starts after the wait
	kaelClock_sleep(waitTime);
	return 0;
}

ktime_t kaelClock_getTick(const KaelClock *clock){
	return clock->tickLow;
}

ktime_t kaelClock_getTickHigh(const KaelClock *clock){
	return clock->tickHigh;
}
