/**
 * @file tui.c
 * 
 * @brief Implementation, Text User Interface tools
 */
#include "kaelygon/book/tui.h"

/**
 * @brief Offsets to get fore-/background colors low and high
 * tens place. 3*10 + ansiBlue = ansiFGLow blue
 */
uint8_t AnsiModValue[4] = { 
	3, //ansiFGLow
	4, //ansiFGHigh
	7, //ansiBGLow
	9, //ansiBGHigh
};

/**
 * @brief Encode ansi color into a single byte
 */
char kaelTui_decodeStyle(const uint8_t mod, const uint8_t color, const uint8_t style){
	KaelTui_ansiCode code = {
		.mod = mod,
		.color = color,
		.style = style
	};
	return code.byte;
}

/**
 * @brief decode ansi color escape sequence e.g \x1b[1;37m from KaelTui_ansiCode.byte
 * 
 * @warning No NULL termination Nor bound checking
 * 
 * example: \esc[underline];[ansiBGLow][blue]m = "\esc4;74m"
 * @return number of bytes written
 * 
 */
uint8_t kaelTui_encodeStyle(uint8_t *escSeq, const uint8_t ansiByte, uint16_t offset){
	KAEL_ASSERT(escSeq!=NULL);

	if ( ansiByte == ansiReset ) {
		escSeq[offset+0]='\x1b';
		escSeq[offset+1]='[';
		escSeq[offset+2]='0';
		escSeq[offset+3]='m';
		return 4;
	}

	KaelTui_ansiCode code = {.byte=ansiByte};
	escSeq[offset+0]='\x1b';
	escSeq[offset+1]='[';
	escSeq[offset+2]=code.style+'0';
	escSeq[offset+3]=';';
	escSeq[offset+4]=AnsiModValue[code.mod]+'0';
	escSeq[offset+5]=code.color+'0';
	escSeq[offset+6]='m';
	return 7;
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




//------ Push to buffer  ------

/**
 * @brief Print white spaces
*/
void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount){
	KAEL_ASSERT(rowBuf!=NULL);
	uint8_t batches = spaceCount/rowBuf->size;
	uint8_t len    = spaceCount - batches * rowBuf->size;

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

void kaelTui_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);
	//Check if rowBuf has enough space for ansiCode
	kaelTui_printFullBuf(rowBuf, ansiLength);
	rowBuf->pos+=kaelTui_encodeStyle(rowBuf->s, rawByte, rowBuf->pos);
	return;
}

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
 * @brief Push escSeq Move terminal cursor
*/
void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row){
	char escSeq[sizeof("\033[65535;65535H")]; 
	uint8_t len = snprintf(escSeq, sizeof(escSeq), "\033[%u;%uH", row+1, col+1);
	kaelTui_pushChar(rowBuf, escSeq, len); //Exclude null byte
}
