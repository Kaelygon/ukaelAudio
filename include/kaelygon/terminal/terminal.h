//./include/kaelygon/terminal/terminal.g
//Text User Interface
#pragma once

#include <stdint.h>
#include "kaelygon/string/string.h"


struct KaelTui{
	KaelStr rowBuf; //row buffer to write uncompressed
	uint16_t rowBufSize;

	//iterate through shapeBuf till type=0
	uint16_t page;
//	KaelBook *book;

	uint16_t canvasSize[2];
	uint8_t charSize; //max bytes per character

	uint16_t padding[2]; //padding to fit 0 width characters
	uint16_t virtualSize[2]; //padding+canvasSize
	
	uint16_t viewSize[4]; //terminal emulator size
	uint16_t viewPos[2]; //shifted view (depends on cursor position)

	uint16_t cursor[2];
	uint8_t quitFlag;

	uint8_t requestRefresh;
	uint8_t debugLines;
};

typedef struct KaelTui KaelTui;

uint8_t kaelTui_alloc(KaelTui *tui);
void kaelTui_free(KaelTui *tui);

void kaelTui_clearTerm();
void kaelTui_rawmode(uint8_t state);
void kaelTui_getKeyPressStr(KaelStr *keyStr);

void kaelTui_signalQuit(int signal __attribute__((unused)));

uint8_t kaelTui_getQuitFlag(const KaelTui *tui);
void kaelTui_setQuitFlag(KaelTui *tui, const uint8_t state);

void kaelTui_drawCanvas(KaelTui *tui);
