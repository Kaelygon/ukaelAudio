/**
 * @file kaelMacros.c
 * 
 * global kaelMacros implementation file
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "kaelygon/global/kaelMacros.h"

// --- Debug implementation ---
#if KAEL_DEBUG==1

#include "kaelygon/string/string.h"



//Debug constants

//string indices
enum KaelDebug_strIndices{
	KAELDEBUG_ERROR_STR,
	KAELDEBUG_NOTE_STR,
	KAELDEBUG_STR_COUNT
};

static const uint16_t KAELDEBUG_INFO_STR_LENGTH = 256U;
typedef struct{
	KaelStr infoStr[KAELDEBUG_STR_COUNT];
} KaelDebug;

//global data, used only during debugging
extern KaelDebug *_GLOBAL_DEBUG;


KaelDebug *_GLOBAL_DEBUG = NULL;
static const char _KAELDEBUG_DEFAULT_STR[2][10] = {
    " BOF DEBUG",
    " BOF INFO\0"
};

//--- alloc free ---
uint8_t kaelDebug_allocGlobal(){
    _GLOBAL_DEBUG = malloc(sizeof(KaelDebug));
    if(_GLOBAL_DEBUG==NULL){
        printf("Error in global debug string.\n");
        return KAEL_ERR_NULL;
    }
    uint8_t code = KAEL_SUCCESS;
    uint8_t outCode = code;

    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        code = kaelStr_alloc(&_GLOBAL_DEBUG->infoStr[i], KAELDEBUG_INFO_STR_LENGTH);
        if(code){outCode=code;}
	    code = kaelStr_setCstr(&_GLOBAL_DEBUG->infoStr[i], _KAELDEBUG_DEFAULT_STR[i]);
        if(code){outCode=code;}
    }

    return outCode;
}

void kaelDebug_freeGlobal(){
    if(_GLOBAL_DEBUG==NULL){return;}
    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        kaelStr_free(&_GLOBAL_DEBUG->infoStr[i]);
    }
    free(_GLOBAL_DEBUG);
}

//--- Functions called by macros ---

uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note) {
    if(ptr==NULL){
        if(_GLOBAL_DEBUG==NULL){
            return 1;}
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "NULL%s_%s ", ptrName, note);
        buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
        kaelStr_pushCstr(&_GLOBAL_DEBUG->infoStr[KAELDEBUG_ERROR_STR], buffer);

        printf("%s",buffer);
        return KAEL_ERR_NULL;
    }
    return KAEL_SUCCESS;
}

void kaelDebug_storeNote(const char *note){
    if(_GLOBAL_DEBUG==NULL){
        return;}
    kaelStr_pushCstr(&_GLOBAL_DEBUG->infoStr[KAELDEBUG_NOTE_STR], note);
}

//--- Other ---

uint8_t kaelDebug_printInfoStr(){
    if(_GLOBAL_DEBUG==NULL){
        printf("Error printing debug\n");
        return KAEL_ERR_NULL;
    }
    uint8_t outCode = KAEL_SUCCESS;
	printf("/********************\n");
    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        kaelStr_print(&_GLOBAL_DEBUG->infoStr[i]);
	    uint8_t code = kaelStr_setCstr(&_GLOBAL_DEBUG->infoStr[i], _KAELDEBUG_DEFAULT_STR[i]);
        if(code){outCode=code;}

        printf("\n");
    }
	printf("\n********************/\n\n");
    return outCode;
}


#else
// --- Release implementation ---
//If any

#endif