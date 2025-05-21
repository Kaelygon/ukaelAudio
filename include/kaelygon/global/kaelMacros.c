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

typedef struct{
	FILE *log;
    pthread_mutex_t logMutex;
} KaelDebug;

KaelDebug *kaelDebug = NULL;

//--- alloc free ---
uint8_t kaelDebug_alloc(const char *programName){
    kaelDebug = malloc(sizeof(KaelDebug));
    if(kaelDebug==NULL){
        printf("Failed to alloc kaelDebug\n");
        return KAEL_ERR_NULL;
    }

    if (pthread_mutex_init(&kaelDebug->logMutex, NULL) != 0) {
        printf("Failed to init logMutex\n");
        free(kaelDebug);
        return KAEL_ERR_ALLOC;
    }

    kaelDebug->log = fopen("./generated/kael.log","a");
    if(kaelDebug->log==NULL){
        pthread_mutex_destroy(&kaelDebug->logMutex);
        free(kaelDebug);
        return KAEL_ERR_ALLOC;
    }

    time_t now = time(NULL);
    char *dateStr = ctime(&now);
    if(dateStr != NULL) {
        fprintf(kaelDebug->log, "###### %s ######\n", programName);    
        fprintf(kaelDebug->log, "%s\n", dateStr);
    }

    return KAEL_SUCCESS;
}

void kaelDebug_free(){
    if(kaelDebug==NULL){return;}
    fflush(kaelDebug->log);
    fclose(kaelDebug->log);
    pthread_mutex_destroy(&kaelDebug->logMutex);
    free(kaelDebug);
}

//--- Functions called by macros ---

uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note) {
    if(ptr==NULL){
        if(kaelDebug==NULL){
            return KAEL_ERR_NULL;
        }
        pthread_mutex_lock(&kaelDebug->logMutex);
            fprintf(kaelDebug->log, "NULL%s_%s\n", ptrName, note);
        pthread_mutex_unlock(&kaelDebug->logMutex);
        return KAEL_ERR_NULL;
    }
    return KAEL_SUCCESS;
}

void kaelDebug_storeNote(const char *note){
    if(kaelDebug==NULL){
        return;
    }
    pthread_mutex_lock(&kaelDebug->logMutex);
        fprintf(kaelDebug->log, note);
    pthread_mutex_unlock(&kaelDebug->logMutex);
}



#else
// --- Release specific implementation ---

    //--- dummy alloc/free ---
    //Dummy arguments that we don't have to define guard every test file 
    uint8_t kaelDebug_alloc(__attribute__((unused)) const char *dummy){
        return KAEL_SUCCESS;
    }

    void kaelDebug_free(){
        fflush(stdout);
    }

    uint8_t kaelDebug_nullCheck(const void* ptr){
        return ptr==NULL ? 1 : 0;
    }

#endif