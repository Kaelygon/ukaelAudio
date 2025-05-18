
#include <stdlib.h>
#include<unistd.h>

#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"

void unit_kaelTuiPrintPage(){

	KaelTui_ansiCode testColor1 = { .mod=AnsiModValue[ansiBGHigh], .color=ansiMagenta, .style=ansiBold };
	KaelTui_ansiCode testColor2 = { .mod=AnsiModValue[ansiFGHigh], .color=ansiBlack, .style=ansiBold };
	KaelTui_ansiCode testColor3 = { .mod=AnsiModValue[ansiBGHigh], .color=ansiBlue, .style=ansiBold };
	
	uint8_t testShapeString[] = {
		markerStyle,testColor3.byte,markerJump,16,
		markerStyle,testColor1.byte,markerStyle,testColor2.byte,
		'0','1','2','3','4','5','6','7',
		'a',markerJump,6,'b',
		markerStyle,testColor3.byte,markerJump,16,
		'c','d','e','f','g','h','i','j',
		'\0',
	};

	KaelTui_rowBuffer rowBuf = {
		.readPtr = NULL,
		.pos = 0,
		.size = 256
	};
	uint8_t testBuffer[256]={0};
	rowBuf.s = testBuffer;

	KaelBook book={0};
	book.size[0]=32;
	book.size[1]=24;
	
	book.viewPos[0]=0;
	book.viewPos[1]=0;
	
	book.rowBuf=&rowBuf;
	kaelTree_alloc(&book.page, sizeof(KaelBook_page));
	kaelTree_alloc(&book.drawQueue, sizeof(KaelBook_shape*));

	KaelBook_page testPage={0};
	testPage.style = testColor1.byte;
	//virtual page size
	testPage.size[0] = 128;
	testPage.size[1] = 24;

	kaelTree_alloc(&testPage.shape, sizeof(KaelBook_shape));

	//Generate shapes
	KaelBook_shape tmpShape = {
		.pos = {4,3},
		.size = {8,5}, //Can't exceed canvas bounds
		.string = testShapeString,
	};
	if(tmpShape.string==NULL){
		return;
	}

	tmpShape.pos[0]=21;
	tmpShape.pos[1]=15;

	for(uint16_t i=0;i<4;i++){
		tmpShape.pos[0]+=tmpShape.size[0];
		tmpShape.pos[1]+=tmpShape.size[1];
		kaelTree_push(&testPage.shape, &tmpShape);
	}

	kaelTree_push(&book.page, &testPage);
	kaelTui_switchPage(&book,0);

	uint64_t startTime = __rdtsc();

	//animate scroll
	for(uint16_t i=0;i<32;i++){
		kaelTui_drawQueue(&book);
		kaelTui_switchPage(&book,0);
		book.viewPos[0]=i;
		book.viewPos[1]=i/2;
		kaelTui_drawQueue(&book);
		usleep(100000);
	}

	uint64_t endTime = __rdtsc();

	kaelTui_freePage(&testPage);
	kaelTree_free(&book.drawQueue);
	kaelTree_free(&book.page);

	uint8_t ansiLen = 8;
	uint8_t ansiResetColor[ansiLen];
	memset(ansiResetColor,0,ansiLen);
	kaelTui_encodeStyle(ansiResetColor, ansiReset, 0);
	printf("%s\ntime %lu\n", ansiResetColor, endTime-startTime);
	
}

int main() {

	unit_kaelTuiPrintPage();

	return 0;
}


