//./include/kaelygon/string/string.h
// Null terminated string implementation
// Declarations
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

typedef struct KaelStr {
	char *s;
	uint16_t size; //allocated bytes
	uint16_t end; //null byte index
}KaelStr;


uint8_t kaelStr_alloc(KaelStr *kstr, uint16_t bytes);
void kaelStr_free(KaelStr *kstr);
uint8_t kaelStr_resize(KaelStr *kstr, const uint16_t bytes);

uint8_t kaelStr_appendCstr(KaelStr *dest, const char *src);
uint8_t kaelStr_appendKstr(KaelStr *dest, const KaelStr *src);

uint8_t kaelStr_pushCstr(KaelStr *dest, const char *src);
uint8_t kaelStr_pushKstr(KaelStr *dest, KaelStr *src);

uint8_t kaelStr_toCstr(char *dest, const KaelStr *src);

uint16_t kaelStr_compareCstr(const KaelStr *kstr1, const void *key2);
uint16_t kaelStr_compareKstr(const KaelStr *kstr1, const KaelStr *kstr2);

uint8_t kaelStr_setCstr(KaelStr *kstr, const char *src);
uint8_t kaelStr_setKstr(KaelStr *kstr1, const KaelStr *kstr2);

uint8_t kaelStr_clear(KaelStr *kstr);
uint8_t kaelStr_setEnd(KaelStr *kstr, uint16_t end);

char* kaelStr_getCharPtr(const KaelStr *kstr);
uint16_t kaelStr_getSize(const KaelStr *kstr);
uint16_t kaelStr_getEnd(const KaelStr *kstr);

uint8_t kaelStr_fillCstr(KaelStr *dest, const char *src);
uint8_t kaelStr_print(const KaelStr *kstr);

void kaelStr_reverseKstr(KaelStr *kstr);

