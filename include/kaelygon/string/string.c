/** 
 * @file string.h
 * 
 * @brief Null terminated general purpose safer string
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "kaelygon/kaelMacros.h" //NULL_CHECK
#include "kaelygon/string/string.h"
#include "kaelygon/math/math.h" //kaelMath_*

KaelStr *kaelStr_alloc(uint16_t bytes) {
	KaelStr *tmpKstr = (KaelStr *)malloc(sizeof(KaelStr));
	if (NULL_CHECK(tmpKstr)) { return NULL; }
	bytes = kaelMath_max(bytes,1); //minimum 1 byte allocation
	tmpKstr->s = (char *)malloc(bytes * sizeof(char));
	if (NULL_CHECK(tmpKstr->s)) { free(tmpKstr); return NULL; }
	tmpKstr->size = bytes;
	kaelStr_setEnd(tmpKstr,0);
	return tmpKstr;
}

void kaelStr_free(KaelStr **kstr){
	if(NULL_CHECK(*kstr)) {return;}
	free((*kstr)->s);
	(*kstr)->s = NULL;
    free((*kstr));
	*kstr = NULL;
	return;
}

uint8_t kaelStr_realloc(KaelStr *kstr, const uint16_t bytes) {
	uint8_t code=KAEL_SUCCESS;
	if(NULL_CHECK(kstr))	{return KAEL_ERR_ARG;}
	char *tmpKstr = realloc(kstr->s, bytes * sizeof(char));
	if (NULL_CHECK(tmpKstr)) { return KAEL_ERR_MEM; }
	if(kstr->end >= bytes){ //truncate if resizing down
		code = kaelStr_setEnd(kstr, bytes-1);
	}
	kstr->size=bytes;
	kstr->s=tmpKstr;
	return code;
}

//Append const char* to KaelStr. Excess is truncated
uint8_t kaelStr_appendCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest)||NULL_CHECK(src))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(dest->s))	{return KAEL_ERR_MEM;}
	uint8_t code=KAEL_SUCCESS;
	uint16_t appendAmount = (dest->size-1) - dest->end; //remaining space
	uint16_t srcSize = strlen(src);
	if(appendAmount < (dest->size-1)){code=KAELSTR_WARN_TRUNCATED;} //mark as trunacted if src doesn't fit in dest
	appendAmount = kaelMath_min(appendAmount,srcSize); //which one is smaller
	memcpy( dest->s+dest->end, src, appendAmount*sizeof(char) ); //copy excluding the null
	dest->end+=appendAmount;
	dest->s[dest->end]='\0'; //null terminate
	return code;
}

//Append const char* to KaelStr
uint8_t kaelStr_appendKstr(KaelStr *dest, const KaelStr *src){
	if(NULL_CHECK(dest)||NULL_CHECK(src))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(dest->s))	{return KAEL_ERR_MEM;}
	uint8_t code=KAEL_SUCCESS;
	uint16_t appendAmount = (dest->size-1) - dest->end; //remaining space
	if(appendAmount < (src->size-1)){code=KAELSTR_WARN_TRUNCATED;}
	appendAmount = kaelMath_min(appendAmount,src->end);
	memcpy( dest->s+dest->end, src->s, appendAmount*sizeof(char) ); //copy excluding the null
	dest->end+=appendAmount;
	dest->s[dest->end]='\0'; //null terminate
	return code;
}

//Insert src to left of dest, truncating right side
uint8_t kaelStr_pushCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest)||NULL_CHECK(src))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(dest->s))	{return KAEL_ERR_MEM;}

	uint16_t srcSize=kaelMath_min(strlen(src),dest->size-1); //how much of src is copied (excl null byte)
	uint16_t copySize=(dest->size-1) - srcSize; //how much of the dest is copied (excl null byte)
	memmove(dest->s + srcSize, dest->s, copySize*sizeof(char)); //cp dest after where src null byte will be
	memcpy(dest->s, src, srcSize*sizeof(char)); //cp src to start of dest
	uint16_t newEnd=dest->end+srcSize;
	kaelStr_setEnd(dest,newEnd); //sets null byte
	return KAEL_SUCCESS;
}

//Convert to C style string assuming *dest has the space for it
uint8_t kaelStr_toCstr(char *dest, const KaelStr *src){
	if(NULL_CHECK(dest)||NULL_CHECK(src))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(src->s))	{return KAEL_ERR_MEM;}
	memcpy( dest, src->s, (src->end+1)*sizeof(char) ); //copy including null termination
	return KAEL_SUCCESS;
}


uint8_t kaelStr_fillCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest)||NULL_CHECK(src))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(dest->s))	{return KAEL_ERR_MEM;}
	uint8_t code=KAEL_SUCCESS;

	dest->end=0;
	uint16_t srcSize = kaelMath_min(dest->size-1,strlen(src));
	if(srcSize>1){ 
		uint16_t maxSize = (dest->size-1) - srcSize;
		while(dest->end < maxSize){ //fill until src doesn't fit
			memcpy( dest->s+dest->end, src, srcSize*sizeof(char) );
			dest->end+=srcSize;
		}
		if(dest->end < (dest->size-1)){ //fill remaining with partial src
			kaelStr_appendCstr(dest,src);
		}
		return code;
	}else if(srcSize==1){ //fill with single character
		memset(dest->s, src[0], dest->size*sizeof(char)-1);
	}else{ //fill with null
		memset(dest->s, 0, dest->size*sizeof(char));
		return code;
	}

	kaelStr_setEnd(dest,dest->size-1);
	return code;
}



uint8_t kaelStr_setCstr(KaelStr *dest, const char *src){
	uint8_t code = kaelStr_clear(dest);
	code = kaelStr_appendCstr(dest,src);
	return code;
}

uint8_t kaelStr_setKstr(KaelStr *dest, const KaelStr *src){
	uint8_t code = kaelStr_clear(dest);
	code = kaelStr_appendKstr(dest,src);
	return code;
}

// return 0 if equal. <0 if kstr1 is smaller, >0 if kstr2 is bigger
uint16_t kaelStr_compareKstr(const KaelStr *kstr1, const KaelStr *kstr2){ 
	if(NULL_CHECK(kstr1)||NULL_CHECK(kstr2))		{return KAEL_ERR_ARG;}
	if(NULL_CHECK(kstr1->s)||NULL_CHECK(kstr2->s))		{return KAEL_ERR_MEM;}
	if(kstr1->end!=kstr2->end){return kstr1->end - kstr2->end;}  //returns negative if kstr1 is smaller, positive if kstr2 is smaller
	return memcmp(kstr1->s, kstr2->s, kstr1->end );
}

// return 0 if equal. <0 if kstr1 is smaller, >0 if key2 is bigger
uint16_t kaelStr_compareCstr(const KaelStr *kstr1, const void *key2){ 
	if(NULL_CHECK(kstr1)||NULL_CHECK(key2))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(kstr1->s)){return KAEL_ERR_MEM;}
	uint16_t key2Len=strlen(key2);
	if(kstr1->end!=key2Len){return kstr1->end - key2Len;}
	return memcmp(kstr1->s, key2, key2Len); 
}

//Move null termination to start
uint8_t kaelStr_clear(KaelStr *kstr){
	kaelStr_setEnd(kstr,0);
	return KAEL_SUCCESS;
}

//set null termination at most to the last allocated byte
uint8_t kaelStr_setEnd(KaelStr *kstr, uint16_t end){
	if(NULL_CHECK(kstr))	{return KAEL_ERR_ARG;}
	if(NULL_CHECK(kstr->s))	{return KAEL_ERR_MEM;}
	if(kstr->size==0){return KAELSTR_WARN_TRUNCATED;}
	kstr->end = kaelMath_min(kstr->size-1,end);
	kstr->s[kstr->end]='\0';
	return KAEL_SUCCESS;
}

char* kaelStr_getCharPtr(const KaelStr *kstr){
	if(NULL_CHECK(kstr))	{return NULL;}
	return kstr->s;
}

uint16_t kaelStr_getSize(const KaelStr *kstr){
	if(NULL_CHECK(kstr))	{return 0;}
	return kstr->size;
}

uint16_t kaelStr_getEnd(const KaelStr *kstr){
	if(NULL_CHECK(kstr))	{return 0;}
	return kstr->end;
}

uint8_t kaelStr_print(const KaelStr *kstr){
	if(NULL_CHECK(kstr))	{return KAEL_ERR_ARG;}
	printf(kstr->s);
	return KAEL_SUCCESS;
}