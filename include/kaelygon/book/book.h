/**
 * @file book.h
 * 
 * @brief Header, Print TUI pages
 */
#pragma once

#include <stdio.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"
#include "kaelygon/book/tui.h"

#include "krle/krleBase.h"
#include "krle/krleText.h"

typedef union {
	struct {
		uint8_t length   : 4;
		uint8_t bright   : 1;
		uint8_t color    : 3;
	};
	uint8_t byte;
}KaelBook_pixel;

//What way should the string be drawn?
typedef enum{
	drawMode_default	= 0,

	//Pack pixel stripes in 1 byte. [3bit : color, 1bit bright, 4bit : length]
	drawMode_pixel,	
}KaelBook_drawMode;

//Not much reason to add length yet. 
//One benefit would be partially hidden shape drawing if length is known, 
//but does it outweigh the need to track the strlen?
typedef struct{
	uint8_t *string; //data byte string
	uint8_t ownsString;
	uint16_t pos[2]; //col/row position  (book space)
	uint16_t size[2];
	uint8_t drawMode; 
}KaelBook_shape;

typedef struct{
	KaelTree shape; //Vector like dynamic data
}KaelBook_page;

typedef struct{
	KaelTree page; //Vector like dynamic data. Book handles page frees
	uint16_t viewPos[2]; //viewport origin offset (book space)
	uint16_t size[2]; //viewport size
	uint16_t index; //Page index

	KaelTui_rowBuffer rowBuf; //print row buffer
	KaelTree drawQueue; //list of shape POINTERS to be printed
}KaelBook;





//------ Alloc / Free ------
uint8_t kaelBook_allocBook(KaelBook *book, const uint16_t viewWidth, const uint16_t viewHeight, uint8_t *stringBuffer, const uint16_t printBufferSize);
void kaelBook_freeBook(KaelBook *book);

void kaelBook_freePage(KaelBook_page *page);
void kaelBook_allocPage(KaelBook_page *page);



//------ drawMode_pixel ------
KaelBook_pixel kaelBook_encodePixel(uint8_t color, uint8_t bright, uint8_t length);
KaelBook_pixel kaelBook_decodePixel(uint8_t byte);

void kaelBook_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte);
void kaelBook_pushMovInShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col, uint16_t row);

Krle_ansiStyle kaelBook_pixelToStyle(KaelBook_pixel pixel);

//------ Panning ------
void kaelBook_scrollRows(KaelBook *book, uint16_t scrollCount, uint16_t scrollUp);
void kaelBook_scrollCols(KaelBook *book, uint16_t scrollCount, uint16_t scrollLeft);



//------ Shape col/row visibility conditions  ------
uint8_t kaelBook_isShapeInRows(KaelBook_shape *shapePtr, uint16_t rowY0, uint16_t rowY1 );
uint8_t kaelBook_isShapeInView(KaelBook *book, KaelBook_shape *shapePtr);

uint8_t kaelBook_isColInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeCol);
uint8_t kaelBook_isRowInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow);

uint8_t kaelBook_isAboveShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow);
uint8_t kaelBook_isBelowShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow);
KaelBook_shape *kaelBook_getShapeAt(KaelBook *book, uint16_t viewCol, uint16_t viewRow);



//------ Get shapePtr ------
KaelBook_shape *kaelBook_getShapeAt(KaelBook *book, uint16_t viewCol, uint16_t viewRow);

