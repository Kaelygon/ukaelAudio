//./include/kaelygon/clock/clock.h
#pragma once

#define CLOCK_SPEED_HZ 3700000000U // 36999640471U
typedef uint64_t ktime_t;

typedef struct{
	ktime_t timeUnit; // how many units one second is. e.g. 1000=milliseconds
	ktime_t unitCycles; // how many cpu cycles one unit is
	ktime_t frameStartTime; // time (units) when last frame was rendered

	uint16_t frameCounter;
	uint16_t frameRate; //text display refresh rate (Hz)
	ktime_t unitsPerFrame; //how many units betweeen updates
}KaelClock;

void kaelClock_init(KaelClock *clock);

ktime_t kaelClock_timeUnits(const KaelClock clock);
void kaelClock_sleepUnits(const KaelClock clock, ktime_t sleepTime);
ktime_t kaelClock_sync(KaelClock *clock);
ktime_t kaelClock_getFrame(const KaelClock *clock);

