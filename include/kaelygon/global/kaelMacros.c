/**
 * @file kaelMacros.c
 * 
 * global kaelMacros implementation file
 * 
 */


#include "kaelygon/global/kaelMacros.h"


// ------ Shared implementation ------

//Extern variables. Later these should be part of opaque type Audio "class"
uint16_t AUDIO_BUFFER_SIZE = 256U; 
uint16_t AUDIO_SAMPLE_RATE = 32768U;



// ------ Debug implementation ------
#if KAEL_DEBUG==1

//Debug constants

//string indices
enum KaelDebug_strIndices{
	KAELDEBUG_ERROR_STR,
	KAELDEBUG_NOTE_STR,
	KAELDEBUG_STR_COUNT
};

static const uint16_t _debugStrLength = 256U;
typedef struct{
	KaelStr infoStr[KAELDEBUG_STR_COUNT];
} KaelDebug;

//global data, used only during debugging
extern KaelDebug *GLOBAL_DEBUG;


KaelDebug *GLOBAL_DEBUG = NULL;
static const char _defaultDebugStr[2][10] = {
    " BOF DEBUG",
    " BOF INFO\0"
};

//--- alloc free ---
uint8_t kaelDebug_allocGlobal(){
    GLOBAL_DEBUG = malloc(sizeof(KaelDebug));
    if(GLOBAL_DEBUG==NULL){
        printf("Error in global debug string.\n");
        return KAEL_ERR_NULL;
    }
    uint8_t code = KAEL_SUCCESS;
    uint8_t outCode = code;

    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        code = kaelStr_alloc(&GLOBAL_DEBUG->infoStr[i], _debugStrLength);
        if(code){outCode=code;}
	    code = kaelStr_setCstr(&GLOBAL_DEBUG->infoStr[i], _defaultDebugStr[i]);
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
        if(GLOBAL_DEBUG==NULL){
            return 1;}
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "NULL%s_%s ", ptrName, note);
        buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
        kaelStr_pushCstr(&GLOBAL_DEBUG->infoStr[KAELDEBUG_ERROR_STR], buffer);

        printf("%s",buffer);
        return KAEL_ERR_NULL;
    }
    return KAEL_SUCCESS;
}

void kaelDebug_storeNote(const char *note){
    if(GLOBAL_DEBUG==NULL){
        return;}
    kaelStr_pushCstr(&GLOBAL_DEBUG->infoStr[KAELDEBUG_NOTE_STR], note);
}

//--- Other ---

uint8_t kaelDebug_printInfoStr(){
    if(GLOBAL_DEBUG==NULL){
        printf("Error printing debug\n");
        return KAEL_ERR_NULL;
    }
    uint8_t outCode = KAEL_SUCCESS;
	printf("/********************\n");
    for(uint8_t i=0; i<KAELDEBUG_STR_COUNT; i++){
        kaelStr_print(&GLOBAL_DEBUG->infoStr[i]);
	    uint8_t code = kaelStr_setCstr(&GLOBAL_DEBUG->infoStr[i], _defaultDebugStr[i]);
        if(code){outCode=code;}

        printf("\n");
    }
	printf("\n********************/\n\n");
    return outCode;
}


#else
// --- Release specific implementation ---
//If any

#endif