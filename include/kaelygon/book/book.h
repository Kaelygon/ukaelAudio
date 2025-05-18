/**
 * @file book.h
 * 
 * @brief Header, Print TUI pages
 */
#pragma once

#include "kaelygon/treeMem/tree.h"
#include <x86intrin.h> 

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include <stdio.h>

extern uint8_t AnsiModValue[4];

/**
 * @brief fancy string print buffer
 */
typedef struct{
	uint8_t* s; //buffer address 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const uint8_t* readPtr; //string char index being read
}KaelTui_RowBuffer;

typedef struct{
	uint8_t *string; //data byte string
	uint16_t pos[2]; //col by row
	uint16_t size[2];
	uint16_t readHead;
	uint8_t lastStyle;
	uint8_t jumpsRemaining;
}KaelBook_shape;

typedef struct{
	KaelTree shape; //Vector like dynamic data
	uint16_t size[2]; //virtual size
	uint8_t style;  //default style
}KaelBook_page;

typedef struct{
	KaelTree page; //Vector like dynamic data
	uint16_t pos[2]; //viewport position of currentrly printed character
	uint16_t size[2]; //viewport size
	uint16_t index; //Page index
	uint16_t scroll;

	KaelTui_RowBuffer *rowBuf; //print row buffer
	KaelTree shapePtrList; //list of shapes on current row
}KaelBook;

//------ Ansi escape sequence ------

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


char kaelTui_decodeAnsiEsc(const uint8_t mod, const uint8_t color, const uint8_t style);
uint8_t kaelTui_encodeAnsiEsc(uint8_t *escSeq, const uint8_t ansiByte, uint16_t offset);

//------ Print shape ------


void kaelTui_freePage(KaelBook_page *page);

void kaelTui_printPage( KaelBook *book);

void unit_kaelTuiPrintPage();