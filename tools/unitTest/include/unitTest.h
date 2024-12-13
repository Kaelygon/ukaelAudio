/**
 * @file unitTest.h
 * 
 * @brief unit test helper functions
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "kaelygon/kaelMacros.h"
#include "./kaelTreeMemUnit.h"
#include "./kaelRandUnit.h"
#include "./kaelTerminalUnit.h"


void unitTest_allocGlobalStr(uint16_t length){
	KAEL_DEBUG_STR=kaelStr_alloc(length);
	KAEL_INFO_STR =kaelStr_alloc(length);

	kaelStr_setCstr(KAEL_DEBUG_STR, "Debug Str");
	kaelStr_setCstr(KAEL_INFO_STR,  "Info Str");
}

void unitTest_freeGlobalStr(){
	kaelStr_free(&KAEL_INFO_STR);

	kaelStr_free(&KAEL_DEBUG_STR);
}

void unitTest_printDebugStr(){
	printf("/********************\n");
	kaelStr_print(KAEL_INFO_STR);
	printf("\n");
	kaelStr_print(KAEL_DEBUG_STR);
	printf("\n********************/\n\n");

	kaelStr_setCstr(KAEL_DEBUG_STR, "Debug Str");
	kaelStr_setCstr(KAEL_INFO_STR,  "Info Str");
}