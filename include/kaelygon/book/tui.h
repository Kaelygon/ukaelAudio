/**
 * @file tui.h
 * 
 * @brief Header, Text User Interface tools
 */
#pragma once

#include <stdio.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "krle/krleBase.h"

//--- Constant values ---

extern const char *kaelTui_constChar[];
extern const uint8_t kaelTui_constCharLen[];

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

//------ Push to Row Bufffer ------

void kaelTui_printRowBuf(KaelTui_rowBuffer *rowBuf);

//Push strings
void kaelTui_pushChar(KaelTui_rowBuffer *rowBuf, const char *string, const uint8_t bytes);
void kaelTui_pushConstChar(KaelTui_rowBuffer *rowBuf, uint16_t index);
void kaelTui_oneArgEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t arg1, char type);
void kaelTui_twoArgEscSeq(KaelTui_rowBuffer *rowBuf, uint16_t arg1, uint16_t arg2, char type);

//Used in drawing
void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount);

//Panning
void kaelTui_pushScroll(KaelTui_rowBuffer *rowBuf, uint8_t scrollCount, uint8_t scrollUp);
void kaelTui_pushLimitScroll(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row);

//Cursor move
void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row);

