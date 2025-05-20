/**
 * @file tui.c
 * 
 * @brief Header, Text User Interface tools
 */
#pragma once

#include "kaelygon/treeMem/tree.h"

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include <stdio.h>

//Enumerates *kaelTui_escSeq[]
typedef enum{
	escSeq_clear,
	escSeq_clearRow,
}KaelTui_escSeqIndex;

extern const char *kaelTui_escSeq[]; 
extern const uint8_t kaelTui_escSeqLen[];

//Enumerates kaelTui_ansiMod[4]
typedef enum{
	ansiFGLow,
	ansiFGHigh,
	ansiBGLow,
	ansiBGHigh,
}KaelTui_ansiMod;

extern uint8_t kaelTui_ansiMod[4];

/**
 * @brief fancy string print buffer
 */
typedef struct{
	uint8_t* s; //buffer address 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const uint8_t* readPtr; //string char index being read
}KaelTui_rowBuffer;

//------ Ansi escape sequence ------

typedef enum {
	ansiResetLength = 4,
	ansiMaxLength 	= 8, //Maximum bytes decoded ansi esc seq can take
}KaelTui_ansiConst;

typedef enum {
	/*Font style*/
	ansiNone			= 0,
	ansiBold 		= 1,  
	ansiUnderline 	= 4, 
	ansiBlink 		= 5, 
	ansiReverse 	= 7, 
	ansiReset 		= 0xFF, // Normally 0 but that's reserved for NULL
	//ansiHidden	= 8, overflow. Use ansiWhite space or space instead
}KaelTui_ansiGlyph;

typedef enum {
	/*Font color*/

	/*Foreground MOD_COL to get ansi code*/
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
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
 * 
 * Formatting in string
 * | [marker]		| [data byte]     |
 * | -----------  | --------------- |
 * | markerStyle	| [ansiCode.byte] | //Print ansi font style esc seq
 * | markerSpace	| [space count]   | //Print white space
 * | markerJump	| [jump count]   | //Space to character without overdraw
 * 
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

//------ Row Bufffer ------

void kaelTui_printRowBuf(KaelTui_rowBuffer *rowBuf);

void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount);
void kaelTui_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte);
void kaelTui_pushChar(KaelTui_rowBuffer *rowBuf, const char *string, const uint8_t bytes);
void kaelTui_pushScroll(KaelTui_rowBuffer *rowBuf, uint8_t scrollCount, uint8_t scrollUp);

void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row);