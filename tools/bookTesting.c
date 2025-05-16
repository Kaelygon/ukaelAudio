
#include <stdlib.h>
#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"

void unit_kaelTuiPrintPage(){

	KaelBook_page page;
	page.vcols = 128;
	page.vrows = 24;

	kaelTree_alloc(&page.shape, sizeof(KaelBook_shape));

	//Generate shapes
	uint16_t strLen = 33;
	KaelBook_shape tmpShape = {
		.pos = {4,1},
		.size = {8,4}, //Can't exceed canvas bounds
		.readHead = 0,
		.lastStyle = ansiReset,
		.jumpsRemaining = 0,
	};
	tmpShape.string = calloc(strLen, sizeof(uint8_t));
	if(tmpShape.string==NULL){
		return;
	}
	
	KaelTui_ansiCode magenta = { .mod=AnsiModValue[ansiBGHigh], .color=ansiMagenta, .style=ansiBold };
	tmpShape.string[0] = markerStyle;
	tmpShape.string[1] = magenta.byte;
	
	const char *tmpStr = "abcdefghij\0";
	memcpy(tmpShape.string+2,tmpStr,12);
	uint8_t curLen = strlen((char*)tmpShape.string);
	tmpShape.string[curLen+0] = markerJump;
	tmpShape.string[curLen+1] = 4;
	tmpShape.string[curLen+2] = 'a';
	tmpShape.string[curLen+3] = markerJump;
	tmpShape.string[curLen+4] = 5;
	tmpShape.string[curLen+5] = 'b';
	tmpShape.string[curLen+6] = markerJump;
	tmpShape.string[curLen+7] = 64;
	tmpShape.string[curLen+10] = '\0';
	curLen = strlen((char*)tmpShape.string);
	

	kaelTree_push(&page.shape, &tmpShape);
	tmpShape.pos[0]=16;
	tmpShape.pos[1]=16;
	kaelTree_push(&page.shape, &tmpShape);

	KaelTui_RowBuffer rowBuf = {
		.s = (uint8_t[32]){0},
		.readPtr = NULL,
		.pos = 0,
		.size = 32
	};

	uint64_t startTime = __rdtsc();
	kaelTui_printPage(&page, &rowBuf);
	uint64_t endTime = __rdtsc();

	free(tmpShape.string);

	kaelTui_freePage(&page);

	uint8_t ansiLen = 8;
	uint8_t ansiResetColor[ansiLen];
	memset(ansiResetColor,0,ansiLen);
	kaelTui_encodeAnsiEsc(ansiResetColor, ansiReset, 0);
	printf("%s\ntime %lu\n", ansiResetColor, endTime-startTime);
	
}

int main() {

	unit_kaelTuiPrintPage();

	return 0;
}


