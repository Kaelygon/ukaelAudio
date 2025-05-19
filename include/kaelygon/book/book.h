/**
 * @file book.h
 * 
 * @brief Header, Print TUI pages
 */
#pragma once

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/treeMem/tree.h"

#include "kaelygon/math/math.h"
#include "kaelygon/book/tui.h"

#include <stdio.h>

typedef struct{
	uint8_t *string; //data byte string
	uint8_t ownsString;
	uint16_t pos[2]; //col by row
	uint16_t size[2];
}KaelBook_shape;

typedef struct{
	KaelTree shape; //Vector like dynamic data
}KaelBook_page;

typedef struct{
	KaelTree page; //Vector like dynamic data. Book handles page frees
	uint16_t viewPos[2]; //viewport origin offset
	uint16_t size[2]; //viewport size
	uint16_t index; //Page index

	KaelTui_rowBuffer rowBuf; //print row buffer
	KaelTree drawQueue; //list of shape POINTERS to be printed
}KaelBook;

//------ Alloc / Free ------

uint8_t kaelBook_allocBook(KaelBook *book, const uint16_t viewWidth, const uint16_t viewHeight, const uint16_t printBufferSize);
void kaelBook_freeBook(KaelBook *book);

void kaelBook_freePage(KaelBook_page *page);
void kaelBook_allocPage(KaelBook_page *page);

//------ High level book manipulation ------

void kaelBook_switchPage(KaelBook *book, uint16_t index);
void kaelBook_drawQueue(KaelBook *book);
void kaelBook_resetStyle(KaelBook *book);