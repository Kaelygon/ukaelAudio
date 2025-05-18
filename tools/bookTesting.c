
#include <stdlib.h>
#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"

/*

typedef struct{
	KaelTree page; //Vector like dynamic data
	uint16_t pos[2]; //viewport position of currentrly printed character
	uint16_t size[2]; //viewport size
	uint16_t index; //Page index

	KaelTui_RowBuffer *rowBuf; //print row buffer
	KaelTree *shapePtrList; //
}KaelBook;
*/

void unit_kaelTuiPrintPage(){

	KaelTui_ansiCode testColor1 = { .mod=AnsiModValue[ansiBGHigh], .color=ansiMagenta, .style=ansiBold };
	KaelTui_ansiCode testColor2 = { .mod=AnsiModValue[ansiFGHigh], .color=ansiBlack, .style=ansiBold };
	KaelTui_ansiCode testColor3 = { .mod=AnsiModValue[ansiBGHigh], .color=ansiBlue, .style=ansiBold };
	
	uint8_t testShapeString[] = {
		markerStyle,testColor1.byte,markerStyle,testColor2.byte,
		'0','1','2','3','4','5','6','7',
		'a',markerJump,6,'b',
		markerStyle,testColor3.byte,markerJump,8,
		'c','d','e','f','g','h','i','j',
		'\0',
	};

	KaelTui_RowBuffer rowBuf = {
		.readPtr = NULL,
		.pos = 0,
		.size = 256
	};
	uint8_t testBuffer[256]={0};
	rowBuf.s = testBuffer;

	KaelBook book={0};
	book.size[0]=128;
	book.size[1]=24;
	book.rowBuf=&rowBuf;
	kaelTree_alloc(&book.page, sizeof(KaelBook_page));
	kaelTree_alloc(&book.shapePtrList, sizeof(KaelBook_shape*));

	KaelBook_page testPage={0};
	testPage.style = testColor1.byte;
	//virtual page size
	testPage.size[0] = 128;
	testPage.size[1] = 24;

	kaelTree_alloc(&testPage.shape, sizeof(KaelBook_shape));

	//Generate shapes
	KaelBook_shape tmpShape = {
		.pos = {4,2},
		.size = {8,4}, //Can't exceed canvas bounds
		.string = testShapeString,
		.readHead = 0,
		.lastStyle = ansiReset,
		.jumpsRemaining = 0,
	};
	if(tmpShape.string==NULL){
		return;
	}
	
	for(uint16_t i=0;i<4;i++){
		kaelTree_push(&testPage.shape, &tmpShape);
		tmpShape.pos[0]+=8;
		tmpShape.pos[1]+=4;
	}
	tmpShape.pos[0]=13;
	tmpShape.pos[1]=2;
	kaelTree_push(&testPage.shape, &tmpShape);
	kaelTree_push(&book.page, &testPage);

	uint64_t startTime = __rdtsc();
	kaelTui_printPage(&book);
	uint64_t endTime = __rdtsc();


	kaelTui_freePage(&testPage);
	kaelTree_free(&book.shapePtrList);
	kaelTree_free(&book.page);

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


