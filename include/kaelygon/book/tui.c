/**
 * @file tui.c
 * 
 * @brief Implementation, Text User Interface tools
 */
#include "kaelygon/book/tui.h"


/**
 * @brief List of escape sequences enumerated by KaelTui_escSeqIndex
 */
const char *kaelTui_escSeq[] = {
	"\033[2J",	//escSeq_clear
	"\033[2K"	//escSeq_clearRow
};

/**
 * @brief List of kaelTui_escSeq lengths enumerated by KaelTui_escSeqIndex
 */
const uint8_t kaelTui_escSeqLen[] = {
	5,	//escSeq_clear
	5	//escSeq_clearRow
};

/**
 * @brief List of ansi style modifiers enumerated by KaelTui_ansiModIndex
 */
uint8_t kaelTui_ansiMod[4] = { 
	3, //ansiFGLow
	9, //ansiFGHigh
	4, //ansiBGLow
	10, //ansiBGHigh
};

uint8_t kaelTui_u16ToString(uint16_t value, char *buf) {
	uint8_t i = 0;
	char tmp[5];
	do{
		tmp[i++] = '0' + (value % 10);
		value /= 10;
	}while(value!=0);
	//reverse
	for (uint8_t j=0; j<i; j++){
		buf[j] = tmp[i-j-1];
	}
	return i;
}

/**
 * @brief Encode ansi color into a single byte
 */
KaelTui_ansiStyle kaelTui_encodeStyle(const uint8_t modIndex, const uint8_t color, const uint8_t style){
	KaelTui_ansiStyle code = {
		.mod	 = modIndex	& 0b11,
		.color = color		& 0b111,
		.style = style		& 0b111
	};
	return code;
}

/**
 * @brief decode ansi color escape sequence e.g \x1b[1;37m from KaelTui_ansiStyle.byte
 * 
 * @warning No NULL termination Nor bound checking
 * 
 * example: \esc[underline];[ansiBGLow][blue]m = "\esc4;74m"
 * @return number of bytes written
 * 
 */
uint8_t kaelTui_styleToString(char *escSeq, const KaelTui_ansiStyle ansiStyle, uint16_t offset){
	KAEL_ASSERT(escSeq!=NULL);
	char *readHeadStart = escSeq+offset;
	char *readHead = readHeadStart;

	if ( ansiStyle.byte == ansiReset ) {
		*readHead++='\x1b';
		*readHead++='[';
		*readHead++='0';
		*readHead++='m';
		return readHead - readHeadStart;
	}

	*readHead++='\x1b';
	*readHead++='[';
	*readHead++=ansiStyle.style+'0';
	*readHead++=';';
	readHead+= kaelTui_u16ToString(kaelTui_ansiMod[ansiStyle.mod], readHead); // The only +1 digit value
	*readHead++=ansiStyle.color+'0';
	*readHead++='m';

	return readHead - readHeadStart;
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
	kaelTui_printFullBuf(rowBuf, ansiMaxLength);
	KaelTui_ansiStyle style = {.byte = rawByte};
	rowBuf->pos+=kaelTui_styleToString((char*)rowBuf->s, style, rowBuf->pos);
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
	char *readHead = escSeq;
	*readHead++ = '\033';
	*readHead++ = '[';

	//Terminal emulators coordinates are offset by col+1, row+1 
	 readHead+= kaelTui_u16ToString(row+1, readHead);
	*readHead++ = ';';
	 readHead+= kaelTui_u16ToString(col+1, readHead); 
	*readHead++ = 'H';

	kaelTui_pushChar(rowBuf, escSeq, readHead - escSeq);
}

/**
 * @brief Scroll terminal
*/
void kaelTui_pushScroll(KaelTui_rowBuffer *rowBuf, uint8_t scrollCount, uint8_t scrollUp){
	char escSeq[sizeof("\033[255S")]; 
	char *readHead = escSeq;
	*readHead++ = '\033';
	*readHead++ = '[';
	readHead+= kaelTui_u16ToString(scrollCount, readHead);
	if(scrollUp){
		*readHead++ = 'S';
	}else{
		*readHead++ = 'T';
	}

	kaelTui_pushChar(rowBuf, escSeq, readHead - escSeq);
}