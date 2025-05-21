// ./include/kaelygon/kaelMacros.h
/**
 * @file kaelMacros.h
 * 
 * @brief Global macros and variables, ideally only for the debugging phase
*/
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

extern uint16_t AUDIO_BUFFER_SIZE;
extern uint16_t AUDIO_SAMPLE_RATE;

//CMake should generate this macro 'KAEL_DEBUG 1'
//code-oss won't know this so this decides which define branches are grayed out
#ifndef KAEL_DEBUG
	#define KAEL_DEBUG 1
#endif


//------ All targets shared variables ------

#include "kaelygon/global/kaelMacrosShared.h"


//------ Debug and Release target exclusives ------

#if KAEL_DEBUG==1
	#include "kaelygon/global/target/debugTarget.h"

#else 
	#include "kaelygon/global/target/releaseTarget.h"

#endif