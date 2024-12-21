/**
 * @file clock.h
 * 
 * Simulate 32768 hz clock
 * Has multiple implementations put togehter
 * 
 * kaelClock_[variant]_[function]();
 * 
*/

#pragma once

#include <stdint.h>
#include "kaelygon/math/math.h"

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/clock/clockShared.h"

//Use 
#if CLOCK_USING_RDTSC
	#include "kaelygon/clock/variant/rdtscClock.h"
#endif

ktime_t kaelClock_time();
void kaelClock_sleep(ktime_t cycles);
void kaelClock_init(KaelClock *clock);
ktime_t kaelClock_sync(KaelClock *clock);
ktime_t kaelClock_getTick(const KaelClock *clock);
ktime_t kaelClock_getTickHigh(const KaelClock *clock);