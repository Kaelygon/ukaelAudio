/**
 * @file releaseTarget.h
 * 
 * @brief kaelMacros implementation header for CMake RELEASE target
 * 
 */

#pragma once

#include "kaelygon/global/kaelMacrosShared.h"

#include <stdint.h>
#include <stdio.h>

//Outside debugging, only null check without printing
//Optimizer should be able to parse out unused parts
#define KAEL_MACRO_ARGS2(_1, _2, NAME, ...) NAME

//Assert ignored in release
#define KAEL_ASSERT(...) ((void)0) 


//Null checks should still act like normally
#define NULL_CHECK_ARGS1(ptr) ( kaelDebug_nullCheck(ptr) )
#define NULL_CHECK_ARGS2(ptr, dummy) ( kaelDebug_nullCheck(ptr) )

//appends errors to KAEL_DEBUG_STR, returns 1 if ptr is NULL, otherwise 0
#define NULL_CHECK(...) KAEL_MACRO_ARGS2(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)

#define KAEL_ERROR_NOTE(dummy) ((void)0)

uint8_t kaelDebug_alloc(const char *programName);
void kaelDebug_free();

uint8_t kaelDebug_nullCheck(const void* ptr);