// ./include/kaelygon/kaelMacros.h
/**
 * @file kaelMacros.h
 * 
 * @brief Global macros and variables, ideally only for the debugging phase
*/
#pragma once

//--- Shared globals ---

#ifndef CHAR_BIT
	#define CHAR_BIT 8
#endif

//128 to 255 errors, 127 to 1 warnings, 0=success
typedef enum {
	 //General
	KAEL_SUCCESS			= 0,
	KAEL_ERR_NULL			= 128,

	 //KaelMem
	KAEL_ERR_FULL			= 131,

	 //KaelStr
	KAELSTR_WARN_TRUNCATED	= 127,
	KAEL_ERR_ARG			= 129,
	KAEL_ERR_MEM			= 130
}Kael_infoCode;


//--- Debug and release exclusive ---

//CMake should generate KAEL_DEBUG 1
#ifndef KAEL_DEBUG
	#define KAEL_DEBUG 1
#endif

#if KAEL_DEBUG==1
	#include "kaelygon/global/kaelDebug.h"
#else 
	#include "kaelygon/global/kaelRelease.h"
#endif


