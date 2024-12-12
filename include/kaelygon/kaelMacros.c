// ./include/kaelygon/kaelMacros.h

#include <stdio.h>
#include <stdint.h>
#include "kaelygon/kaelMacros.h"
#include "kaelygon/string/string.h"
#include "kaelygon/clock/clock.h"

KaelStr *KAEL_DEBUG_STR = NULL;
KaelStr *KAEL_INFO_STR = NULL;

uint8_t KAEL_NULL_CHECK(const char *ptrName) {
    if(KAEL_DEBUG_STR==NULL){return KAEL_ERR_NULL;}
    char buffer[16+8];
    snprintf(buffer, sizeof(buffer), "NULL%s ", ptrName);
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
    kaelStr_pushCstr(KAEL_DEBUG_STR, buffer);
    return 1;
}

uint8_t KAEL_NULL_CHECK_NOTE(const char *ptrName, const char *note) {
    if(KAEL_DEBUG_STR==NULL){return KAEL_ERR_NULL;}
    char buffer[32]; // Adjust size as needed
    snprintf(buffer, sizeof(buffer), "NULL%s %s ", ptrName, note);
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
    kaelStr_pushCstr(KAEL_DEBUG_STR, buffer);
    return 1;
}

uint8_t KAEL_STORE_NOTE(const char *note){
    if(KAEL_INFO_STR==NULL){return KAEL_ERR_NULL;}
    kaelStr_pushCstr(KAEL_INFO_STR, note);
    return 1;
}