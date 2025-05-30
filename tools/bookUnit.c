/**
 * @file bookUnit.c
 * 
 * @brief Test program for book.c
 * 
 * TODO: Pixel mode is currently broken as I am drafting KRLE standard
 * TODO: Create similar functions like KREL->Pixels in krleTGA.c and declare them in krleBase.c 
 * TODO: All colors aren't displayed correctly, The old pixel draw method has collisions with new KRLE standard
 */

#include <stdlib.h>
#include <unistd.h>

#include "kaelygon/book/book.h"
#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/book/bookDraw.h"

const char KAELBOOK_TEST_PATTERN[] = "abcdefghijklmnopqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

KaelBook_shape kaelBook_genCheckerboard(uint16_t width, uint16_t height, uint16_t col, uint16_t row, uint16_t ratio[2]){
	KaelBook_shape shape = {0};
	shape.size[0]  = width;
	shape.size[1]  = height;
	shape.pos[0]   = col;
	shape.pos[1]   = row;

	Krle_ansiStyle blackText = krle_encodeStyle( ANSI_NORMAL, ANSI_FG, ANSI_BLACK, ANSI_BOLD );
	Krle_ansiStyle highCol = krle_encodeStyle  ( ANSI_BRIGHT, ANSI_BG, ANSI_WHITE, ANSI_BOLD );
	Krle_ansiStyle lowCol  = krle_encodeStyle  ( ANSI_BRIGHT, ANSI_BG, ANSI_BLUE , ANSI_BOLD );
	Krle_ansiStyle curCol = highCol;

	uint8_t lastState = 3;
	uint16_t index=0;

	KaelTree tmpString;
	kaelTree_alloc(&tmpString, sizeof(uint8_t));

	//rounded to nearest full squares
	size_t cellsTotal = ((width)/ratio[0]+1)*((height)/ratio[1]+1);
	// half is filled with chars + 2 byte jump markers in every row of every second square + 2 byte style marker every square every row 
	size_t byteEstimate = width*height/2 + cellsTotal*ratio[1] + 2*cellsTotal*ratio[1] + 3; 
	kaelTree_reserve(&tmpString, byteEstimate); 

	kaelTree_push(&tmpString,&(uint8_t){KRLE_TEXT_STYLE});
	kaelTree_push(&tmpString,&blackText.byte);

	for(uint16_t j=0; j<height; j++){
		for(uint16_t i=0; i<width; i++){
			//1=chars, 0=color 
			uint8_t state = ((i / ratio[0] + j / ratio[1]) % 2) == 0;

			if(lastState!=state){
				//Swap style
				curCol.byte = state ? highCol.byte : lowCol.byte;
				kaelTree_push(&tmpString,&(uint8_t){KRLE_TEXT_STYLE});
				kaelTree_push(&tmpString,&curCol.byte);
				KAEL_ASSERT(curCol.byte!='\0', "Illegal value");
			}

			if(state){
				kaelTree_push(&tmpString, &KAELBOOK_TEST_PATTERN[index]);
				index = index < (sizeof(KAELBOOK_TEST_PATTERN)-2) ? index+1 : 0;
			}else{
				//Colored jumps at beginning of every second square 
				uint8_t spaceCount = kaelMath_min(ratio[0], width-i);
				spaceCount+= spaceCount==0;
				if(spaceCount>2){
					//Printing marker is more efficient
					kaelTree_push(&tmpString,&(uint8_t){KRLE_TEXT_SPACE});
					kaelTree_push(&tmpString,&(uint8_t){spaceCount});
				}else{
					//Pritning space is more efficient
					for(uint16_t si=0; si<spaceCount; si++){
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

KaelBook_shape kaelBook_genPixel(uint16_t width, uint16_t height, uint16_t col, uint16_t row, uint8_t pixelLength){
	KaelBook_shape shape = {0};
	shape.size[0]  = width;
	shape.size[1]  = height;
	shape.pos[0]   = col;
	shape.pos[1]   = row;
	shape.drawMode	= drawMode_pixel;

	pixelLength = pixelLength&0b1111;
	uint16_t mixByte = 0;

	KaelTree tmpString;
	kaelTree_alloc(&tmpString, sizeof(uint8_t));
	
	size_t byteEstimate = width*height+8;
	byteEstimate = pixelLength ? byteEstimate/pixelLength : byteEstimate/8;
	kaelTree_reserve(&tmpString, byteEstimate);

	size_t pixelCount=0;

	for(uint16_t j=0; j<height; j++){
		for(uint16_t i=0; i<width; i++){
			mixByte+=0b00010000; //Increment colors place

			KaelBook_pixel pixel = {.byte = mixByte };
			pixel.length = pixel.length == 0 ? 1 : pixel.length;
			pixel.length = pixelLength ? pixelLength : pixel.length;

			pixelCount+=pixel.length;
			kaelTree_push(&tmpString,&(uint8_t){pixel.byte});
			KAEL_ASSERT(pixel.byte!='\0', "Illegal value");
			
			if(pixelCount>=width*height){break;}
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
	uint16_t checkerWidth  = 32;
	uint16_t checkerHeight = 16;
	KaelBook_shape checkerBoard = kaelBook_genCheckerboard(checkerWidth, checkerHeight, 0,0, (uint16_t[2]){4,2});
	kaelTree_push(&testPage.shape, &checkerBoard);
	checkerBoard.ownsString=0; //Keep track of ownership to avoid double free

	checkerBoard.pos[0] = bookWidth	- checkerWidth;
	checkerBoard.pos[1] = bookHeight	- checkerHeight;
	kaelTree_push(&testPage.shape, &checkerBoard);

	KaelBook_shape *firstShape = kaelTree_begin(&testPage.shape);
	uint16_t pixelDrawCol = firstShape->pos[0] + firstShape->size[0];
	uint16_t pixelDrawWidth  = bookWidth	- checkerWidth;
	uint16_t pixelDrawHeight = checkerHeight;
	KaelBook_shape pixelDraw = kaelBook_genPixel(pixelDrawWidth, pixelDrawHeight, pixelDrawCol, 0, 2);
	kaelTree_push(&testPage.shape, &pixelDraw);

	return testPage;
}

void unit_kaelBook_scramblePixels(KaelBook *book){
	KaelBook_shape *shapePtr = kaelBook_getShapeAt(book, 48, 0); //get shape at 48,0
	if(shapePtr!=NULL && shapePtr->drawMode==drawMode_pixel){
		//Scramble data
		uint8_t *readHead = shapePtr->string;
		while(readHead[0]){
			switch(readHead[0]){
				case KRLE_PIXEL_JUMP:
					readHead++;
					break;
				case 0:
					break;
				default:
					// 
					KaelBook_pixel pixel = kaelBook_decodePixel((uint8_t)readHead[0]);
					pixel.color++;
					pixel.bright = pixel.color==0 ? !pixel.bright : pixel.bright;
					readHead[0] = pixel.byte;
					readHead++;
					break;
			}
		}
		//add to draw queue
		kaelTree_push(&book->drawQueue, &shapePtr);
	}
}

void unit_kaelBook(){

	KaelBook book;
	uint8_t stringBuffer[1024] = {0};
	kaelBook_allocBook(&book, 64, 16, stringBuffer, sizeof(stringBuffer));

	KaelBook_page testPage = kaelBook_genTestPage(64, 32);
	kaelTree_push(&book.page, &testPage);

	kaelBook_switchPage(&book,0);

	book.viewPos[0] = 0;
	book.viewPos[1] = 0;

	kaelBook_queueViewShapes(&book);
	kaelBook_drawQueue(&book);

	for(uint16_t j=1; j<5; j++){
		for(uint16_t i=0; i<16; i++){

			kaelBook_scrollRows(&book,i%4==0,j%2);

			unit_kaelBook_scramblePixels(&book);

			kaelBook_drawQueue(&book);
			usleep(100000);
		}
	}

	kaelBook_resetStyle(&book);
	kaelBook_freeBook(&book);
}

int main() {
	kaelDebug_alloc("kael_bookUnit");

	unit_kaelBook();

	kaelDebug_free();

	return 0;
}


