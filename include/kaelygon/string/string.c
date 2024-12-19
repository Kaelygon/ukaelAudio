/** 
 * @file string.h
 * 
 * @brief Null terminated general purpose safer string
*/
#include "kaelygon/string/string.h"

uint8_t kaelStr_alloc(KaelStr *kstr, uint16_t bytes) {
	if(NULL_CHECK(kstr))	{
		return KAEL_ERR_NULL;}
	bytes = kaelMath_max(bytes,1); //minimum 1 byte allocation for null byte

	kstr->s = (char *)malloc(bytes * sizeof(char));
	if (NULL_CHECK(kstr->s)){
		return KAEL_ERR_ALLOC; }

	kstr->size = bytes;
	kaelStr_setEnd(kstr,0);
	return KAEL_SUCCESS;
}

void kaelStr_free(KaelStr *kstr){
	if(NULL_CHECK(kstr)) {return;}
	free(kstr->s);
	kstr->s = NULL;
}

uint8_t kaelStr_resize(KaelStr *kstr, const uint16_t bytes) {
	if(NULL_CHECK(kstr) || NULL_CHECK(kstr->s)){
		return KAEL_ERR_NULL;}
	char *tmpKstr = realloc(kstr->s, bytes * sizeof(char));
	if (NULL_CHECK(tmpKstr)){
		return KAEL_ERR_ALLOC; }
	kstr->size=bytes;
	kstr->s=tmpKstr;
		
	uint8_t code=KAEL_SUCCESS;
	if(kstr->end >= bytes){ //truncate if resizing down
		code = kaelStr_setEnd(kstr, bytes-1);
	}
	return code;
}

//Append const char* to KaelStr. Excess is truncated
uint8_t kaelStr_appendCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest) || NULL_CHECK(src) || NULL_CHECK(dest->s)){
			return KAEL_ERR_NULL;}

	uint16_t appendAmount = (dest->size-1) - dest->end; //remaining space
	uint16_t srcSize = strlen(src);
	appendAmount = kaelMath_min(appendAmount,srcSize); //which one is smaller

	memcpy( dest->s+dest->end, src, appendAmount*sizeof(char) ); //copy excluding the null
	dest->end+=appendAmount;
	dest->s[dest->end]='\0'; //null terminate
	return KAEL_SUCCESS;
}

//Append const char* to KaelStr
uint8_t kaelStr_appendKstr(KaelStr *dest, const KaelStr *src){
	if(NULL_CHECK(dest) || NULL_CHECK(src) || NULL_CHECK(dest->s)){
			return KAEL_ERR_NULL;}
	KAEL_ASSERT(dest!=src, "Overlapping buffer");

	uint16_t appendAmount = (dest->size-1) - dest->end; //remaining space
	appendAmount = kaelMath_min(appendAmount,src->end);
	
	memcpy( dest->s+dest->end, src->s, appendAmount*sizeof(char) ); //copy excluding the null
	dest->end+=appendAmount;
	dest->s[dest->end]='\0'; //null terminate
	return KAEL_SUCCESS;
}

//Insert src to left of dest, truncating right side
uint8_t kaelStr_pushCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest) || NULL_CHECK(src) || NULL_CHECK(dest->s)){
			return KAEL_ERR_NULL;}

	uint16_t srcSize=kaelMath_min(strlen(src),dest->size-1); //how much of src is copied (excl null byte)
	uint16_t copySize=(dest->size-1) - srcSize; //how much of the dest is copied (excl null byte)
	memmove(dest->s + srcSize, dest->s, copySize*sizeof(char)); //cp dest after where src null byte will be
	memcpy(dest->s, src, srcSize*sizeof(char)); //cp src to start of dest
	uint16_t newEnd=dest->end+srcSize;
	kaelStr_setEnd(dest,newEnd); //sets null byte
	return KAEL_SUCCESS;
}

uint8_t kaelStr_pushKstr(KaelStr *dest, KaelStr *src){
	char* srcPtr = src->s;
	uint8_t code = kaelStr_pushCstr(dest, srcPtr);
	return code;
}

//Convert to C style string assuming *dest has the space for it
uint8_t kaelStr_toCstr(char *dest, const KaelStr *src){
	if(NULL_CHECK(dest) || NULL_CHECK(src) || NULL_CHECK(src->s)){
		return KAEL_ERR_NULL;}

	memcpy( dest, src->s, (src->end+1)*sizeof(char) ); //copy including null termination
	return KAEL_SUCCESS;
}


uint8_t kaelStr_fillCstr(KaelStr *dest, const char *src){
	if(NULL_CHECK(dest) || NULL_CHECK(src) || NULL_CHECK(dest->s))	{
		return KAEL_ERR_NULL;}

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
		return KAEL_SUCCESS;
	}else if(srcSize==1){ //fill with single character
		memset(dest->s, src[0], dest->size*sizeof(char)-1);
	}else{ //fill with null
		memset(dest->s, 0, dest->size*sizeof(char));
		return KAEL_SUCCESS;
	}

	kaelStr_setEnd(dest,dest->size-1);
	return KAEL_SUCCESS;
}



uint8_t kaelStr_setCstr(KaelStr *dest, const char *src){
	uint8_t code = kaelStr_clear(dest);
	uint8_t code2 = kaelStr_appendCstr(dest,src);
	code = code2>127 ? code2 : code;
	return code;
}

uint8_t kaelStr_setKstr(KaelStr *dest, const KaelStr *src){
	uint8_t code = kaelStr_clear(dest);
	uint8_t code2 = kaelStr_appendKstr(dest,src);
	code = code2>127 ? code2 : code;
	return code;
}

// return 0 if equal. <0 if kstr1 is smaller, >0 if kstr2 is bigger
uint16_t kaelStr_compareKstr(const KaelStr *kstr1, const KaelStr *kstr2){ 
	if(NULL_CHECK(kstr1) || NULL_CHECK(kstr2) || NULL_CHECK(kstr1->s) || NULL_CHECK(kstr2->s)){
		return KAEL_ERR_NULL;}
	
	if(kstr1->end!=kstr2->end){return kstr1->end - kstr2->end;}  //returns negative if kstr1 is smaller, positive if kstr2 is smaller
	return memcmp(kstr1->s, kstr2->s, kstr1->end );
}

// return 0 if equal. <0 if kstr1 is smaller, >0 if key2 is bigger
uint16_t kaelStr_compareCstr(const KaelStr *kstr1, const void *key2){ 
	if(NULL_CHECK(kstr1) || NULL_CHECK(key2) || NULL_CHECK(kstr1->s)){
		return KAEL_ERR_NULL;}

	uint16_t key2Len=strlen(key2);
	if(kstr1->end!=key2Len){return kstr1->end - key2Len;}
	return memcmp(kstr1->s, key2, key2Len); 
}

//Move null termination to start
uint8_t kaelStr_clear(KaelStr *kstr){
	uint8_t code = kaelStr_setEnd(kstr,0);
	return code;
}

//set null termination at most to the last allocated byte
uint8_t kaelStr_setEnd(KaelStr *kstr, uint16_t end){
	if(NULL_CHECK(kstr) || NULL_CHECK(kstr->s)){
		return KAEL_ERR_NULL;}

	kstr->end = kaelMath_min(kstr->size-1,end);
	kstr->s[kstr->end]='\0';
	return KAEL_SUCCESS;
}

char* kaelStr_getCharPtr(const KaelStr *kstr){
	if(NULL_CHECK(kstr)){
		return NULL;}

	return kstr->s;
}

uint16_t kaelStr_getSize(const KaelStr *kstr){
	if(NULL_CHECK(kstr)){
		return 0;}

	return kstr->size;
}

uint16_t kaelStr_getEnd(const KaelStr *kstr){
	if(NULL_CHECK(kstr)){
		return 0;}

	return kstr->end;
}

uint8_t kaelStr_print(const KaelStr *kstr){
	if(NULL_CHECK(kstr)){
		return KAEL_ERR_NULL;}
	fwrite(kstr->s, sizeof(char), kstr->end, stdout); //+1 null terminate
	return KAEL_SUCCESS;
}

/**
 * @brief reverse KaelStr
 * 
 * @param cstr C string
 * @param len length excluding null byte
 */
void kaelStr_reverseCstr(char* cstr, uint16_t len){
	if( len==0 || NULL_CHECK(cstr)){
		return;}
    len-=1;
    for(size_t i=0; i<=len/2; i++){
        char buf=cstr[i];
        cstr[i]=cstr[len-i];
        cstr[len-i]=buf;
    }
}

void kaelStr_reverseKstr(KaelStr *kstr){
	if(NULL_CHECK(kstr)){
		return;}
	kaelStr_reverseCstr(kstr->s, kstr->end);
}