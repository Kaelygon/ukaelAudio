/**
 * @file tui.c
 * 
 * @brief Implementation, Text User Interface tools
 */

#include "kaelygon/book/tui.h"
#include <string.h>


/**
 * @brief List of escape sequences enumerated by KaelTui_escSeqIndex
 */
const char *kaelTui_escSeq[] = {
	"\x1b[2J",	//escSeq_clear
	"\x1b[2K",	//escSeq_clearRow
	"\x1b[0m",	//escSeq_styleReset
	"\x1b[r"		//escSeq_scrollReset
};

/**
 * @brief List of kaelTui_escSeq lengths enumerated by KaelTui_escSeqIndex
 */
const uint8_t kaelTui_escSeqLen[] = {
	4,	//escSeq_clear
	4,	//escSeq_clearRow
	4,	//escSeq_styleReset
	3	//escSeq_scrollReset
};

/**
 * @brief List of ansi style modifiers enumerated by KaelTui_ansiModIndex
 */
const uint8_t kaelTui_ansiMod[4] = { 
	3, //ansiFGLow
	9, //ansiFGHigh
	4, //ansiBGLow
	10, //ansiBGHigh
};





//------ Helper functions ------

/**
 * @brief Convert uint16_t to string and write it to *buf. 
 * 
 * @note No bound checking or NULL termination
 */
uint8_t kaelTui_u16ToString(uint16_t value, char *buf) {
	uint8_t i = 0;
	char tmp[5];
	do{
		tmp[i++] = '0' + (value % 10);
		value /= 10;
	}while(value!=0);
	//reverse
	for(uint8_t j=0; j<i; j++){
		buf[j] = tmp[i-j-1];
	}
	return i;
}


/*
* Ansi reset is normally kaelTui_encodeStyle(0,0,0) but this is reseved for NULL because 
* If rowBuffer string endend in a [markerStyle], it would read 1 byte out of bounds
* This also means that both escSeq_styleReset and kaelTui_encodeStyle(3,7,7)=0xFF are reset
*/

/**
 * @brief Encode ansi color into a single byte
 *
 */ 
KaelTui_ansiStyle kaelTui_encodeStyle(const uint8_t modIndex, const uint8_t color, const uint8_t style){
	KaelTui_ansiStyle code = {
		.mod	 = modIndex	& 0b11,
		.color = color		& 0b111,
		.style = style		& 0b111
	};
	return code;
}





//------ Buffer printing ------

/**
 * @brief Check if rowBuf has at least bytes free space
 */
uint8_t kaelTui_fitsInBuf(KaelTui_rowBuffer *rowBuf, uint16_t bytes){
	KAEL_ASSERT(rowBuf->pos <= rowBuf->size, "rowBuf overflow");
	return (rowBuf->pos + bytes <= rowBuf->size);
}

void kaelTui_printRowBuf(KaelTui_rowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	//read head can't be beyond its size
	KAEL_ASSERT(rowBuf->pos <= rowBuf->size, "rowBuf overflow");
	//Print only up to overwritten part
	fwrite((char *)rowBuf->s, sizeof(char), rowBuf->pos, stdout); 
	rowBuf->pos=0;
	return;
}

/**
 * @brief Print rowBuf if bytes won't fit
 */
void kaelTui_printFullBuf(KaelTui_rowBuffer *rowBuf, uint16_t bytes){
	if( !kaelTui_fitsInBuf(rowBuf,bytes) ){
		kaelTui_printRowBuf(rowBuf);
	}
}





//------ Push constructed escape sequence to rowBuf

/**
 * @brief push 1 argument escape sequence to rowBuf
*/
void kaelTui_oneArgEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t arg1, char type){
	char escSeq[sizeof("\x1b[65535#")]; 
	char *readHead = escSeq;
	*readHead++ = '\x1b';
	*readHead++ = '[';
	readHead+= kaelTui_u16ToString(arg1, readHead);
	*readHead++ = type;

	kaelTui_pushChar(rowBuf, escSeq, readHead - escSeq);
}

/**
 * @brief push 2 argument escape sequence to rowBuf
*/
void kaelTui_twoArgEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t arg1, uint16_t arg2, char type){
	char escSeq[sizeof("\x1b[65535;65535#")]; 
	char *readHead = escSeq;
	*readHead++ = '\x1b';
	*readHead++ = '[';

	//Terminal emulators coordinates are offset by col+1, row+1 
	 readHead+= kaelTui_u16ToString(arg1, readHead);
	*readHead++ = ';';
	 readHead+= kaelTui_u16ToString(arg2, readHead); 
	*readHead++ = type;

	kaelTui_pushChar(rowBuf, escSeq, readHead - escSeq);
}





//------ Push to buffer  ------

/**
 * @brief Push bytes of string to rowbuf
*/
void kaelTui_pushChar(KaelTui_rowBuffer *rowBuf, const char *string, const uint8_t bytes){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(string!=NULL);

	kaelTui_printFullBuf(rowBuf,bytes);
	memcpy((uint8_t*)&rowBuf->s[rowBuf->pos], string, bytes);
	rowBuf->pos+=bytes;
	return;
}

/**
 * @brief Wrapper function to push escSeq array by index
 */
void kaelTui_pushEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t index){
	kaelTui_pushChar(rowBuf, kaelTui_escSeq[index], kaelTui_escSeqLen[index]);
}

/**
 * @brief Push escSeq Move terminal cursor
*/
void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row){
	kaelTui_twoArgEscSeq(rowBuf, row+1, col+1, 'H');
}

/**
 * @brief Limit scrolling to a region
*/
void kaelTui_pushLimitScroll(KaelTui_rowBuffer *rowBuf, uint16_t top, uint16_t bottom){
	kaelTui_twoArgEscSeq(rowBuf, top+1, bottom+1, 'r');
}

/**
 * @brief Print white spaces
*/
void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount){
	KAEL_ASSERT(rowBuf!=NULL);
	uint16_t batches = spaceCount / rowBuf->size;
	uint16_t len    = spaceCount - batches * rowBuf->size;

	kaelTui_printFullBuf(rowBuf,len);
	memset((uint8_t*)&rowBuf->s[rowBuf->pos], ' ', sizeof(uint8_t)*len);
	rowBuf->pos+=len;

	//if spaceCount is larger than rowBuf->size
	for(uint16_t i=0; i<batches; i++){
		kaelTui_printRowBuf(rowBuf);
		memset(rowBuf->s, ' ', rowBuf->size);
		fwrite(rowBuf->s, sizeof(char), rowBuf->size, stdout);
	}
}

/**
 * @brief Scroll terminal
*/
void kaelTui_pushScroll(KaelTui_rowBuffer *rowBuf, uint8_t scrollCount, uint8_t scrollUp){
	if(scrollUp){
		kaelTui_oneArgEscSeq(rowBuf, scrollCount,'S');
	}else{
		kaelTui_oneArgEscSeq(rowBuf, scrollCount,'T');
	}
}

/**
 * @brief Push decoded kaelTui_ansiCode.byte to string  
 */
void kaelTui_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);

	if ( rawByte == ansiReset ) {
		kaelTui_pushChar(rowBuf, kaelTui_escSeq[escSeq_styleReset], kaelTui_escSeqLen[escSeq_styleReset]);
		return;
	}

	KaelTui_ansiStyle ansiStyle = {.byte = rawByte};
	uint16_t arg1 = ansiStyle.style;
	uint16_t arg2 = kaelTui_ansiMod[ansiStyle.mod]*10 + ansiStyle.color;
	kaelTui_twoArgEscSeq(rowBuf, arg1, arg2, 'm');
}