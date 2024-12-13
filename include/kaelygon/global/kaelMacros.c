/**
 * @file kaelMacros.h
 * 
 * global kaelMacros implementation file
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/string/string.h"

#if KAEL_DEBUG==1

KaelDebug *GLOBAL_DEBUG = NULL;

/**
 * @brief Returns default initialization string by enum KaelDebug_strIndices
 */
const char* kaelDebug_getDefaultString(uint8_t stringID){
    switch(stringID){
        case KAELDEBUG_ERROR_STR:
            return " BOF DEBUG";
        case KAELDEBUG_NOTE_STR:
            return " BOF INFO";
        default:
            return NULL;
    }
}

//--- alloc free ---
uint8_t kaelDebug_allocGlobal(){
    GLOBAL_DEBUG = malloc(sizeof(KaelDebug));
    if(GLOBAL_DEBUG==NULL){return KAEL_ERR_MEM;}
    uint8_t code = KAEL_SUCCESS;
    uint8_t outCode = code;

    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        code = kaelStr_alloc(&GLOBAL_DEBUG->infoStr[i], KAELDEBUG_INFO_STR_LENGTH);
        if(code){outCode=code;}
	    code = kaelStr_setCstr(&GLOBAL_DEBUG->infoStr[i], kaelDebug_getDefaultString(i));
        if(code){outCode=code;}
    }

    return outCode;
}

void kaelDebug_freeGlobal(){
    if(GLOBAL_DEBUG==NULL){return;}
    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        kaelStr_free(&GLOBAL_DEBUG->infoStr[i]);
    }
    free(GLOBAL_DEBUG);
}

//--- Functions called by macros ---

uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note) {
    if(ptr==NULL){
        if(GLOBAL_DEBUG==NULL){return 1;}
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "NULL%s_%s ", ptrName, note);
        buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
        kaelStr_pushCstr(&GLOBAL_DEBUG->infoStr[KAELDEBUG_ERROR_STR], buffer);
        return KAEL_ERR_NULL;
    }
    return KAEL_SUCCESS;
}

void kaelDebug_storeNote(const char *note){
    if(GLOBAL_DEBUG==NULL){return;}
    kaelStr_pushCstr(&GLOBAL_DEBUG->infoStr[KAELDEBUG_NOTE_STR], note);
}

//--- Other ---

uint8_t kaelDebug_printInfoStr(){
    if(GLOBAL_DEBUG==NULL){return KAEL_ERR_NULL;}
    uint8_t outCode = KAEL_SUCCESS;
	printf("/********************\n");
    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        kaelStr_print(&GLOBAL_DEBUG->infoStr[i]);
	    uint8_t code = kaelStr_setCstr(&GLOBAL_DEBUG->infoStr[i], kaelDebug_getDefaultString(i));
        if(code){outCode=code;}

        printf("\n");
    }
	printf("\n********************/\n\n");
    return outCode;
}


#endif