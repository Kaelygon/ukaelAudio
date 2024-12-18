// ./include/kaelygon/kaelMacros.h
/**
 * @file kaelMacros.h
 * 
 * @brief Global macros and variables, ideally only for the debugging phase
*/
#pragma once


//CMake should generate KAEL_DEBUG 1
#ifndef KAEL_DEBUG
	#define KAEL_DEBUG 1
#endif

//--- Shared globals ---

#include "kaelygon/global/anyTarget.h"

//--- Debug and release exclusive ---

#if KAEL_DEBUG==1
	#include "kaelygon/global/debugTarget.h"
#else 
	#include "kaelygon/global/releaseTarget.h"
#endif


