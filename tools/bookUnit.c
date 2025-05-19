
#include <stdlib.h>
#include<unistd.h>

#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

const char testCharList[] = "abcdefghijklmnopqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
uint16_t testCharListSize = sizeof(testCharList);


KaelBook_shape kaelBook_genCheckerboard(uint16_t width, uint16_t height, uint16_t col, uint16_t row, uint16_t ratio[2]){
	KaelBook_shape shape;
	shape.string   = NULL;
	shape.ownsString = 0;
	shape.size[0]  = width;
	shape.size[1]  = height;
	shape.pos[0]   = col;
	shape.pos[1]   = row;

	KaelTui_ansiCode highCol = { .mod=AnsiModValue[ansiBGHigh], .color=ansiWhite, .style=ansiBold };
	KaelTui_ansiCode lowCol = { .mod=AnsiModValue[ansiBGHigh], .color=ansiBlue, .style=ansiBold };
	KaelTui_ansiCode curCol = highCol;

	uint8_t lastState = 3;
	uint16_t index=0;

	KaelTree tmpString;
	kaelTree_alloc(&tmpString, sizeof(uint8_t));
	
	//rounded to nearest full squares
	size_t cellsTotal = ((width)/ratio[0]+1)*((height)/ratio[1]+1);
	// half is filled with chars + 2 byte jump markers in every row of every second square + 2 byte style marker every square every row 
	size_t byteEstimate = width*height/2 + cellsTotal*ratio[1] + 2*cellsTotal*ratio[1]; 
	kaelTree_reserve(&tmpString, byteEstimate); 
	for(uint16_t j=0; j<height; j++){
		for(uint16_t i=0; i<width; i++){
			//1=chars, 0=color 
			uint8_t state = ((i / ratio[0] + j / ratio[1]) % 2) == 0;

			if(lastState!=state){
				//Swap style
				curCol.byte = state ? highCol.byte : lowCol.byte;
				kaelTree_push(&tmpString,&(uint8_t){markerStyle});
				kaelTree_push(&tmpString,&curCol.byte);
			}

			if(state){
				kaelTree_push(&tmpString, &testCharList[index]);
				index = index < (testCharListSize-2) ? index+1 : 0;
			}else{
				//Colored jumps at beginning of every second square 
				uint8_t spaceCount = kaelMath_min(ratio[0], width-i);
				if(spaceCount>2){
					//Printing marker is more efficient
					kaelTree_push(&tmpString,&(uint8_t){markerSpace});
					kaelTree_push(&tmpString,&(uint8_t){spaceCount});
				}else{
					//Pritning space is more efficient
					for(uint si=0; si<spaceCount; si++){
						kaelTree_push(&tmpString, &(uint8_t){' '});
					}
				}
				i+=spaceCount-1;
				i = i<width ? i : i%width;
			}
			
			lastState = state;
		}
	}
	//Null terminate
	kaelTree_push(&tmpString,'\0');

	//No free, shape takes ownership of tree.data
	shape.string = (uint8_t*)kaelTree_get(&tmpString, 0);
	shape.ownsString = 1;

	return shape;
}

KaelBook_page kaelBook_genTestPage(uint16_t bookWidth, uint16_t bookHeight){
	KaelBook_page testPage;
	kaelBook_allocPage(&testPage);

	//Create checker board pattern on opposite corners
	uint16_t checkerWidth  = 16;
	uint16_t checkerHeight = 8;
	KaelBook_shape checkerBoard = kaelBook_genCheckerboard(checkerWidth, checkerHeight, 0,0, (uint16_t[2]){4,2});
	kaelTree_push(&testPage.shape, &checkerBoard);
	checkerBoard.ownsString=0; //Keep track of ownership to avoid double free

	checkerBoard.pos[0] = bookWidth	- checkerWidth;
	checkerBoard.pos[1] = bookHeight	- checkerHeight;
	kaelTree_push(&testPage.shape, &checkerBoard);

	return testPage;
}

void unit_kaelBook(){
	KaelBook book;
	kaelBook_allocBook(&book, 64, 32, 256);

	KaelBook_page testPage = kaelBook_genTestPage(64, 32);
	kaelTree_push(&book.page, &testPage);

	kaelBook_switchPage(&book,0);
	kaelBook_drawQueue(&book);
	
	kaelBook_resetStyle(&book);
	kaelBook_freeBook(&book);
}

int main() {
	unit_kaelBook();
	return 0;
}


