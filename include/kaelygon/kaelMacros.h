// ./include/kaelygon/kaelMacros.h
// Global macros and variables, ideally only for the debugging phase

#pragma once

//128 to 255 errors, 127 to 1 warnings, 0=success
typedef enum {
    //General
	KAEL_SUCCESS			= 0,
	KAEL_ERR_NULL			= 132,

    //KaelMem
	KAEL_ERR_FULL			= 131,

    //KaelStr
	KAELSTR_WARN_TRUNCATED	= 127,
	KAEL_ERR_ARG			= 129,
	KAEL_ERR_MEM			= 130
}Kael_infoCode;

#ifndef KAEL_DEBUG
	#define KAEL_DEBUG 1
#endif

#if KAEL_DEBUG==1

	typedef struct KaelStr KaelStr;

	//global KAEL_DEBUG_STR and KAEL_INFO_STR write macros
	//Used only for debugging
	extern KaelStr *KAEL_DEBUG_STR; //Errors
	extern KaelStr *KAEL_INFO_STR; //Testing info

	uint8_t kaelStr_pushCstr(KaelStr *dest, const char *src);

	uint8_t KAEL_NULL_CHECK(const char *ptrName); //DEBUG
	uint8_t KAEL_NULL_CHECK_NOTE(const char *ptrName, const char *note);
	uint8_t KAEL_STORE_NOTE(const char *note); //INFO


	//This is the only way I were able to print variable name instead of adresses
	#define KAEL_GET_MACRO(_1, _2, NAME, ...) NAME

	#define NULL_CHECK_ARGS1(ptr) (           \
		((ptr) == NULL) ?                     \
			(KAEL_NULL_CHECK(#ptr), 1) :      \
			0                                 \
	)

	#define NULL_CHECK_ARGS2(ptr, note) (          	\
		((ptr) == NULL) ?                     		\
			(KAEL_NULL_CHECK_NOTE(#ptr, note), 1) : \
			0                                 		\
	)

	//appends errors to KAEL_DEBUG_STR, returns 1 if ptr is NULL, otherwise 0
	#define NULL_CHECK(...) KAEL_GET_MACRO(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)
#else 
	//Outside debugging, only null check without printing
	#define KAEL_GET_MACRO(_1, _2, NAME, ...) NAME

	#define NULL_CHECK_ARGS1(ptr) ( ((ptr) == NULL) ? 1 : 0 )

	#define NULL_CHECK_ARGS2(ptr, dummy) ( ((ptr) == NULL) ? 1 : 0 )

	#define NULL_CHECK(...) KAEL_GET_MACRO(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)
#endif


#ifndef CHAR_BIT
	#define CHAR_BIT 8
#endif

