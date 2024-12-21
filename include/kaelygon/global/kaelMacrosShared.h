/**
 * @file kaelDefines.h
 * 
 * @brief Header, Global macros that may be defined in other libraries shared between all build targets
 * 
 */

#pragma once

// #define CLOCK_SPEED_HZ 3700000000U
 #define CLOCK_SPEED_HZ 3704954300U

//128 to 255 errors, 127 to 1 warnings, 0=success
typedef enum {
	 //General
	KAEL_SUCCESS			= 0U,
	KAEL_ERR_NULL			= 128U,

	 //KaelMem
	KAEL_ERR_FULL			= 131U,

	 //KaelStr
	KAEL_ERR_ALLOC			= 130U
}Kael_infoCode;