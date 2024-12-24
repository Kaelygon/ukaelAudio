
#include "kaelygon/treeMem/tree.h"
#include <x86intrin.h> 

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include <stdio.h>


//static const uint8_t _puaStart = 0xE0;
//static const uint8_t _puaEnd   = 0xF8;
static const uint8_t _unicodeMask = 0b10000000;

/**
 * @brief fancy string print buffer
 */
typedef struct{
	char* s; //buffer address 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const char* read; //ptr to string being printed
}KaelRowBuffer;


typedef enum {
	/*Style*/
	ansiReset 		= 0, 
	ansiBold 		= 1,  
	ansiUnderline 	= 4, 
	ansiBlink 		= 5, 
	ansiReverse 	= 7, 
	//ansiHidden	= 8, overflow. Use ansiWhite space or jump instead

	/*Foreground MOD_COL to get ansi code*/
	ansiBlack 	= 0,
	ansiRed 		= 1, 
	ansiGreen 	= 2, 
	ansiYellow 	= 3, 
	ansiBlue 	= 4, 
	ansiMagenta = 5, 
	ansiCyan 	= 6, 
	ansiWhite 	= 7, 
} AnsiCode_enum;

typedef enum{
	ansiFGLow, 
	ansiFGHigh, 
	ansiBGLow, 
	ansiBGHigh, 
}AnsiMod_enum;

//Offsets to get fore-/background colors low and high
uint8_t AnsiModValue[4] = {
	30, //ansiFGLow
	40, //ansiFGHigh
	70, //ansiBGLow
	90, //ansiBGHigh
};

/**
 * @brief Ansi color code escape sequence encoded in a single byte
 */
typedef union {
	struct {
		uint8_t mod    : 2;
		uint8_t color  : 3;
		uint8_t style  : 3;
	};
	uint8_t byte;
} AnsiCode;

/**
 * @brief Encode ansi color into a single byte
 */
char encodeAnsiStyle(uint8_t mod, uint8_t color, uint8_t style){
	AnsiCode code = {
		.mod = mod,
		.color = color,
		.style = style
	};
	return code.byte;
}

/**
 * @brief decode ansi color escape sequence e.g \x1b[1;37m from AnsiCode.byte
 */
void decodeAnsiStyle(char *escSeq, char ansiByte){
	KAEL_ASSERT(escSeq!=NULL);
	if (ansiByte == 0) {
		memcpy(escSeq,"\x1b[0m",5);
		return;
	}
	AnsiCode code = {.byte=ansiByte};
	snprintf(escSeq, 10, "\x1b[%d;%dm", code.style, code.color + AnsiModValue[code.mod]);
}

void printRowBuf(KaelRowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	fwrite(rowBuf->s, sizeof(char), rowBuf->pos, stdout); //Print only up to overwritten part
	fflush(stdout);
	rowBuf->pos=0;
}

void printWhitespace(KaelRowBuffer *rowBuf, const uint8_t count){
	KAEL_ASSERT(rowBuf!=NULL);
	for(uint16_t i=0; i<count; i++){ //jump
		if(rowBuf->pos > rowBuf->size){
			printRowBuf(rowBuf);
		}
		rowBuf->s[rowBuf->pos]=' ';
		rowBuf->pos+=1;
	}
}

/**
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
 */
typedef enum{
	MARKER_JUMP = 0b11100000,
	MARKER_STYLE = 0b11100001,
}stringMarker;

/*
	Private Use Area (PUA), 2-byte range: U+E000 to U+F8FF
	Linux ANSI color escape sequences are atrociously long, up to 8 bytes '\x1b'[1;37m
	Let us use unicode PUA BMP range U+E0_00 to U+F8_FF
	Check if byte MSB is 1, 0x80. In this case we have either unicode or PUA sequence
	
	If next byte is 0xE or 0xF, next byte is a marker, otherwise print unicode by the given length 'L', 
	number of leading 1s set the unicode length, [0b1L...0#] [0b10######] [0b10######] ...
	e.g. 0b11110000 has total 4 bytes, the trailing bits are part of the unicode
	
	Since 0xE0 to 0xF8 are 
*/	
void testFancyPrint(const char *textString){

	//Set to very small for testing
	const uint16_t rowBufSize = 10; 
	char rowBufArray[rowBufSize];

	KaelRowBuffer rowBuf = {
		.s = rowBufArray,
		.read = textString,
		.pos = 0,
		.size = rowBufSize
	};

	while( rowBuf.read[0] ){
		char firstByte = rowBuf.read[0];

		uint8_t isUnicode = firstByte & _unicodeMask; //unicode flag is the firstByte leading bit
		if( isUnicode ){
			//Unicode or PUA
			//Parse marker
			switch( (uint8_t)firstByte ){
				case MARKER_JUMP:
					rowBuf.read++; //advance to the first data byte
					printWhitespace(&rowBuf, rowBuf.read[0]);
					rowBuf.read++; //skip data byte
					break;

				case MARKER_STYLE:
					rowBuf.read++;
					char ansiBuf[8];
					decodeAnsiStyle(ansiBuf, rowBuf.read[0]);
					uint8_t ansiBufLen=strlen(ansiBuf);
					if( (rowBuf.pos+ansiBufLen+1) >= rowBufSize ){ //Make sure the string fits
						printRowBuf(&rowBuf);
					}
					memcpy(rowBuf.s+rowBuf.pos,ansiBuf,ansiBufLen); //copy excluding null byte
					rowBuf.pos+=ansiBufLen;
					break;

				default: //Unkown marker or unicode
					//Print unicode
					//Get number of leading 1s by inverting and then counting the leading zeros
					uint8_t unicodeLegth = 0;

					//count leading 1s
					while( (firstByte&0x80) != 0 ){
						firstByte<<=1;
						unicodeLegth++;
					}
					
					if ((rowBuf.pos + unicodeLegth) >= rowBuf.size) {
						printRowBuf(&rowBuf);
					}
					memcpy(rowBuf.s + rowBuf.pos, rowBuf.read, unicodeLegth);
					rowBuf.pos+=unicodeLegth;
					//Increment readhead ensuring we didn't skip null termination
					//in case of invalid length unicode
					while(rowBuf.read[0]){ 
						rowBuf.read++;
					}
					break;
			}
		}else{
			//Write to rowBuf
			rowBuf.s[rowBuf.pos++] = firstByte;
		}

		rowBuf.read++; //Advance to next character

		if( rowBuf.pos >= rowBufSize ){
			printRowBuf(&rowBuf);
		}

	}
	printRowBuf(&rowBuf); //Print remaining buffer

	return;
}









int main() {
	char underLineMagenta = encodeAnsiStyle(ansiFGLow, ansiMagenta, ansiUnderline);

	//Some 16-bit string address from some memory bank
	const unsigned char textString[] = {
		MARKER_STYLE, underLineMagenta, // Underline magenta
		'H','e','l','l','o',
		MARKER_JUMP, 4, // Jump 4 spaces
		' ','W','o','r','l','d','!',
		MARKER_STYLE,ansiReset, // Reset color
		0xf0, 0x9f, 0x98, 0x80, // Raw binary of 0x1f600 since we can't store uint32_t 
		'\n','\0'
	};

	testFancyPrint((char *)textString);

	return 0;
}



