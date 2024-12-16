/**
 * @file kaelDebug.h
 * 
 * @brief kaelMacros implementation header for CMake DEBUG target
 */

#pragma once

#include <assert.h>
#include "kaelygon/string/string.h"

//Select 1 or 2 argument macro
#define KAEL_MACRO_ARGS2(_1, _2, NAME, ...) NAME


//Debug assert
#define KAEL_ASSERT_ARGS1(condition) assert(condition)
#define KAEL_ASSERT_ARGS2(condition, message) assert((condition) && (message))

#define KAEL_ASSERT(...) KAEL_MACRO_ARGS2(__VA_ARGS__, KAEL_ASSERT_ARGS2, KAEL_ASSERT_ARGS1)(__VA_ARGS__)


//Since this is a terminal program, terminal may not be available 
//so debug can be stored in global strings during debugging
#define KAEL_ERROR_NOTE(note) (kaelDebug_storeNote(note))


//Debug NULL

#define NULL_CHECK_ARGS1(ptr) ( kaelDebug_nullCheck(ptr, #ptr, NULL) )
#define NULL_CHECK_ARGS2(ptr, note) ( kaelDebug_nullCheck(ptr, #ptr, note) )

//appends errors to KAEL_DEBUG_STR, returns 1 if ptr is NULL, otherwise 0
#define NULL_CHECK(...) KAEL_MACRO_ARGS2(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)


//Debug constants
enum KaelDebug_const{
	KAELDEBUG_INFO_STR_LENGTH = 256,
};
//string indices
enum KaelDebug_strIndices{
	KAELDEBUG_ERROR_STR,
	KAELDEBUG_NOTE_STR,
	KAELDEBUG_STR_COUNT
};

typedef struct{
	KaelStr infoStr[KAELDEBUG_STR_COUNT];
} KaelDebug;
//global data, used only for debugging
extern KaelDebug *GLOBAL_DEBUG;



uint8_t kaelStr_pushCstr(KaelStr *dest, const char *src);

uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note); //DEBUG
void kaelDebug_storeNote(const char *note); //INFO

uint8_t kaelDebug_allocGlobal();
void kaelDebug_freeGlobal();

uint8_t kaelDebug_printInfoStr();