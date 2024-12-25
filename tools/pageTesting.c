
#include "kaelygon/treeMem/tree.h"
#include <x86intrin.h> 

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include <stdio.h>

/**
 * @brief fancy string print buffer
 */
typedef struct{
	uint8_t* s; //buffer address 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const uint8_t* read; //ptr to string being printed
}KaelTui_RowBuffer;


typedef enum {
	/*Style*/
	ansiReset 		= 0xFF, // we can't have null data
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

	ansiLength 	= 7, //Maximum bytes decoded ansi esc seq can take
} KaelTui_ansiColor;

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

//indices of the table AnsiModValue[]
typedef enum{
	ansiFGLow, 
	ansiFGHigh, 
	ansiBGLow, 
	ansiBGHigh, 
}KaelTui_ansiMod;

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
} KaelTui_ansiCode;

/**
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
 * 
 * Formatting in string
 * | [marker]		| [data byte]     |
 * | -----------  | --------------- |
 * | markerJump	| [jump length]   |
 * | markerStyle	| [ansiCode.byte] |
 * 
 */
typedef enum{
	markerJump = 0b11100000,
	markerStyle = 0b11100001,
}KaelTui_Marker;



//------ Ansi escape sequence ------


/**
 * @brief Encode ansi color into a single byte
 */
char kaelTui_decodeAnsiEsc(const uint8_t mod, const uint8_t color, const uint8_t style){
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
 * @warning No NULL termination  
 * 
 * example: \esc[underline];[ansiBGLow][blue]m = "\esc4;74m"
 * @return number of bytes written
 */
uint8_t kaelTui_encodeAnsiEsc(uint8_t *escSeq, const uint8_t ansiByte, uint16_t offset){
	KAEL_ASSERT(escSeq!=NULL);

	if ( ansiByte == ansiReset ) {
		escSeq[offset++]='\x1b';
		escSeq[offset++]='[';
		escSeq[offset++]='0';
		escSeq[offset++]='m';
		return 4;
	}

	KaelTui_ansiCode code = {.byte=ansiByte};
	escSeq[offset++]='\x1b';
	escSeq[offset++]='[';
	escSeq[offset++]=code.style+'0';
	escSeq[offset++]=';';
	escSeq[offset++]=AnsiModValue[code.mod]+'0';
	escSeq[offset++]=code.color+'0';
	escSeq[offset++]='m';
	return 7;
}



//------ Print shape ------


void kaelTui_printRowBuf(KaelTui_RowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	//Print only up to overwritten part
	fwrite((char *)rowBuf->s, sizeof(char), rowBuf->pos, stdout); 
	rowBuf->pos=0;
	return;
}

/**
 * @brief Print white spaces
*/
void kaelTui_fillJump(KaelTui_RowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(rowBuf!=0);
	rowBuf->read++; //skip the marker
	uint8_t jumpCount=rowBuf->read[0];
	if(jumpCount==0){
		return;  //NULL data
	};

	for(uint16_t i=0; i<jumpCount; i++){ //jump
		if(rowBuf->pos >= rowBuf->size){
			kaelTui_printRowBuf(rowBuf);
		}
		rowBuf->s[rowBuf->pos]=' ';
		rowBuf->pos+=1;
	}

	rowBuf->read+=1; //skip data byte
	return;
}

void kaelTui_printMarkerStyle(KaelTui_RowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(rowBuf!=0);
	rowBuf->read++;
	if(rowBuf->read[0]==0){
		return; //NULL data
	};
	rowBuf->pos+=kaelTui_encodeAnsiEsc(rowBuf->s, rowBuf->read[0], rowBuf->pos);
	return;
}

/**
	@brief print rectangle in viewport

	Private Use Area (PUA), 2-byte range: U+E000 to U+F8FF
	Linux ANSI color escape sequences are atrociously long, up to 8 bytes '\x1b'[1;37m
	Let us use unicode PUA BMP range U+E0_00 to U+F8_FF
	
	Check if the first byte falls in this range it is a marker
	otherwise print unicode by the given length 'L', 
	number of leading 1s set the unicode length, [0b1L...0#] [0b10######] [0b10######] ...
	e.g. 0b11110000 has total 4 bytes, the trailing bits are part of the unicode
	
	Since 0xE0 to 0xF8 are 
*/	
void kaelTui_printShape(const uint8_t *textString){
	if(NULL_CHECK(textString) || textString[0]=='\0'){return;}

	//Set to very small for testing
	const uint16_t rowBufSize = 255; 
	uint8_t rowBufArray[rowBufSize];

	KaelTui_RowBuffer rowBuf = {
		.s = rowBufArray,
		.read = textString,
		.pos = 0,
		.size = rowBufSize
	};

	while( 1 ){
		uint8_t firstByte = rowBuf.read[0];

		//Parse character
		switch( (uint8_t)firstByte ){
			case markerJump:
				//Print number of spaces
				kaelTui_fillJump(&rowBuf);
				break;

			case markerStyle:
				//Ansi style and color encoding
				kaelTui_printMarkerStyle(&rowBuf);
				break;

			case 0: 
				//NULL
				kaelTui_printRowBuf(&rowBuf);
				fflush(stdout);
				return;

			default: 
				//Regular characters and unicode
				rowBuf.s[rowBuf.pos++] = firstByte;
				break;

		}				
		rowBuf.read++; //Advance to next character

		if( rowBuf.pos+ansiLength >= rowBufSize ){
			kaelTui_printRowBuf(&rowBuf);
		}

	}
	//This should never be reached
	KAEL_ASSERT(0,"How did you get here?\n");
	return;
}


//------ Unit test ------


void unit_genRandomShape(char *bigText, uint32_t charCount, uint8_t randNum[3]){
	for(uint32_t i=0; i<charCount; i++){
		uint8_t symbol=0;
		uint8_t legalString=1;

		symbol = kaelRand_lcg24(randNum);
		symbol+= symbol==0;

		if(i%3==0){
			bigText[i++]=(char)markerStyle;
			if(i>=charCount){break;}
			bigText[i]=symbol;

		}else{
			if(legalString){
				uint8_t type = (randNum[1]>>6)&0b11;
				switch(type){
					case 0:
						symbol = symbol*('9'-'0')/255 + '0';
						break;
					case 1:
						symbol = symbol*('Z'-'A')/255 + 'A';
						break;
					case 2:
						symbol = symbol*('z'-'a')/255 + 'a';
						break;
					case 3:
						symbol = ' ';
						break;
				}
			}

			bigText[i]=symbol;
		}
	}
	if(charCount){
		bigText[charCount-1]='\0';
	}
	return;
}


void unit_kaelTuiPrintShape(uint8_t randNum[3]){

	//Far too overkill benchmark
	uint32_t charCount=1U<<20;
	char *bigText = NULL;
	bigText = malloc(charCount);

	unit_genRandomShape(bigText,charCount,randNum);

	uint64_t startTime = __rdtsc();
	kaelTui_printShape((uint8_t *)bigText);
	uint64_t endTime = __rdtsc();
	free(bigText);

	if(0){
		uint8_t ansiLen = 8;
		uint8_t ansiResetColor[ansiLen];
		memset(ansiResetColor,0,ansiLen);
		kaelTui_encodeAnsiEsc(ansiResetColor, ansiReset, 0);
		printf("%stime %lu\n", ansiResetColor, endTime-startTime);

		//print text
		char underLineMagenta = kaelTui_decodeAnsiEsc(ansiFGLow, ansiMagenta, ansiBold);
	
		const uint8_t textString[] = {
			markerStyle,ansiReset, // Reset color
			markerStyle, underLineMagenta, // Underline magenta
			'H','e','l','l','o',
			markerJump, 4, // Jump 4 spaces
			' ','W','o','r','l','d','!',
			markerStyle,ansiReset,
			0xF0, 0x9F, 0x90, 0x89, // Raw binary since we can't store uint32_t 
			'\n','\0'
		};
		
		kaelTui_printShape(textString);
	}
}

int main() {

	uint8_t randNum[3]={31,82,84};
	while(1){
		unit_kaelTuiPrintShape(randNum);
	}

	return 0;
}



