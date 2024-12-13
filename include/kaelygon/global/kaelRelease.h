/**
 * @file kaelRelease.h
 * 
 * @brief Release build macros for CMake OPTIMIZED target
 * 
 */

#pragma once

#include <stdio.h>

//Outside debugging, only null check without printing
//Optimizer should be able to parse out unused parts

#define KAEL_ERROR_NOTE(note) (printf("%s",note))

#define KAEL_GET_MACRO(_1, _2, NAME, ...) NAME
#define NULL_CHECK_ARGS1(ptr) ( ((ptr) == NULL) ? 1 : 0 )
#define NULL_CHECK_ARGS2(ptr, dummy) ( ((ptr) == NULL) ? 1 : 0 )

#define NULL_CHECK(...) KAEL_GET_MACRO(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)