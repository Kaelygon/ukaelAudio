/**
 * @file rdtscClock.h
 * 
 * @brief Header, clock using rdtsc
 * 
 */
#pragma once

#include <unistd.h>
#include <x86intrin.h> 

//Types and definitions that all the implementation variants use
#include "kaelygon/clock/clockShared.h"

void kaelClock_rdtsc_sleep(ktime_t sleepTime);
ktime_t kaelClock_rdtsc_time();