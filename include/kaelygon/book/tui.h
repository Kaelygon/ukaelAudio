/**
 * @file tui.c
 * 
 * @brief Header, Text User Interface tools
 */
#pragma once

#include <stdio.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

/**
 * @brief fancy string print buffer
 */
typedef struct{
	uint8_t* s; //buffer address 
	uint8_t ownsBuffer; //owns .s allocation and it should be freed 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const uint8_t* readPtr; //string char index being read
}KaelTui_rowBuffer;





//------ Ansi escape sequence ------

/**
 * @brief Enumerates *kaelTui_escSeq[] and kaelTui_escSeqLen[]
 */
typedef enum{
	escSeq_clear,
	escSeq_clearRow,
	escSeq_styleReset,
	escSeq_scrollReset
}KaelTui_escSeqIndex;

/**
 * @brief Enumerates kaelTui_ansiMod[4]
 */
typedef enum{
	ansiFGLow,
	ansiFGHigh,
	ansiBGLow,
	ansiBGHigh,
}KaelTui_ansiMod;

typedef enum {
	ansiMaxLength 	= 8, //Maximum bytes decoded ansi esc seq can take
	ansiReset 		= 0xFF, 
}KaelTui_ansiConst;

/**
 * @brief Font style
 */
typedef enum {
	ansiNone			= 0,
	ansiBold 		= 1,  
	ansiUnderline 	= 4, 
	ansiBlink 		= 5, 
	ansiReverse 	= 7, 
	//ansiHidden	= 8, overflow. Use ansiWhite space or space instead
}KaelTui_ansiGlyph;

/**
 * @brief font color
 */
typedef enum {
	ansiBlack 	= 0,
	ansiRed 		= 1, 
	ansiGreen 	= 2, 
	ansiYellow 	= 3, 
	ansiBlue 	= 4, 
	ansiMagenta = 5, 
	ansiCyan 	= 6, 
	ansiWhite 	= 7, 

}KaelTui_ansiColor;

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
}KaelTui_ansiStyle;

/**
 * 
 * Formatting in string
 * | [marker]		| [data byte]     |
 * | -----------  | --------------- |
 * | markerStyle	| [ansiCode.byte] | //Print ansi font style esc seq
 * | markerSpace	| [space count]   | //Print white space
 * | markerJump	| [jump count]   	| //Space to character without overdraw
 * 
 */
/**
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
*/
typedef enum{
	markerStyle = 0xE0,
	markerSpace,
	markerJump,
	markerPlaceholder = 0xF8
}KaelTui_Marker;





//------ Ansi style escape sequence ------

KaelTui_ansiStyle kaelTui_encodeStyle(const uint8_t mod, const uint8_t color, const uint8_t style);
uint8_t kaelTui_styleToString(char *escSeq, const KaelTui_ansiStyle ansiStyle, uint16_t offset);

//------ Push to Row Bufffer ------

void kaelTui_printRowBuf(KaelTui_rowBuffer *rowBuf);

//Push strings
void kaelTui_pushChar(KaelTui_rowBuffer *rowBuf, const char *string, const uint8_t bytes);
void kaelTui_pushEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t index);

//Used in drawing
void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount);
void kaelTui_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte);

//Panning
void kaelTui_pushScroll(KaelTui_rowBuffer *rowBuf, uint8_t scrollCount, uint8_t scrollUp);
void kaelTui_pushLimitScroll(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row);

//Cursor move
void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row);