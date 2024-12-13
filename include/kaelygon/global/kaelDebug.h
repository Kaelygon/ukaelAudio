/**
 * @file kaelDebug.h
 * 
 * @brief Debug build macros for CMake DEBUG target
 */

#pragma once

#include "kaelygon/string/string.h"

//This is the only way I were able to print variable name instead of adresses
#define KAEL_ERROR_NOTE(note) (kaelDebug_storeNote(note))

#define KAEL_GET_MACRO(_1, _2, NAME, ...) NAME
#define NULL_CHECK_ARGS1(ptr) ( kaelDebug_nullCheck(ptr, #ptr, NULL) )
#define NULL_CHECK_ARGS2(ptr, note) ( kaelDebug_nullCheck(ptr, #ptr, note) )

//appends errors to KAEL_DEBUG_STR, returns 1 if ptr is NULL, otherwise 0
#define NULL_CHECK(...) KAEL_GET_MACRO(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)


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