/**
 * @file kaelDefines.h
 * 
 * @brief Global macros that may be defined in other libraries
 * 
 */

#define CLOCK_SPEED_HZ 3700000000U 

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