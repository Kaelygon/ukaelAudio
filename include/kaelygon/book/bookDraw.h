/**
 * @file bookDraw.c
 * 
 * @brief Implementation, High level TUI drawing tools
 * 
 */
#pragma once

#include <stdio.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "kaelygon/book/tui.h"
#include "kaelygon/book/book.h"

#include "krle/krleBase.h"
#include "krle/krleText.h"

//------ High level book manipulation ------

void kaelBook_queueViewShapes(KaelBook *book);
void kaelBook_switchPage(KaelBook *book, uint16_t index);
void kaelBook_drawQueue(KaelBook *book);
void kaelBook_resetStyle(KaelBook *book);