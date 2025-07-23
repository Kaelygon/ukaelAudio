/**
 * @file book.c
 * 
 * @brief Implementation, Print TUI pages
 * 
 * Hierarchy
 * book -> page -> shape -> string
 */

#include "kaelygon/book/book.h"




//------ Alloc / Free ------

/**
 * @brief free page trees
 */
void kaelBook_freePage(KaelBook_page *page){
	while(!kaelTree_empty(&page->shape)){
		KaelBook_shape *curShape = kaelTree_back(&page->shape);
		if(curShape->ownsString){
			free(curShape->string);
		}
		kaelTree_pop(&page->shape);
	}
	kaelTree_free(&page->shape);
}

/**
 * @brief Allocate book
 * 
 * @note Optionally provide *stringBuffer that points to allocated memory that's at least printBufferSize bytes. Otherwise book->rowBuf.s is allocated to heap
 */
uint8_t kaelBook_allocBook(KaelBook *book, const uint16_t viewWidth, const uint16_t viewHeight, uint8_t *stringBuffer, const uint16_t printBufferSize){
	uint8_t code = KAEL_SUCCESS;

	*book = (KaelBook){0};
	book->size[0]    = viewWidth;
	book->size[1]    = viewHeight;

	book->rowBuf = (KaelTui_rowBuffer){0};
	book->rowBuf.size = printBufferSize;

	if(stringBuffer==NULL){
		//Heap allocation
		book->rowBuf.s = calloc(book->rowBuf.size, sizeof(uint8_t));

		code = NULL_CHECK(book->rowBuf.s) ? KAEL_ERR_NULL : code;
		if(NULL_CHECK(book->rowBuf.s)){ 
			code = KAEL_ERR_NULL; 
			goto label_rowBufFail; 
		}

		book->rowBuf.ownsBuffer = 1; //Remember to free itself
	}else{
		//Use provided buffer
		book->rowBuf.s = stringBuffer;
		memset(book->rowBuf.s, 0U, printBufferSize);
		book->rowBuf.ownsBuffer = 0;
	}

	code = kaelTree_alloc(&book->page, sizeof(KaelBook_page));
	if(code != KAEL_SUCCESS){ goto label_pageFail; }

	code = kaelTree_alloc(&book->drawQueue, sizeof(KaelBook_shape));
	if(code != KAEL_SUCCESS){ goto label_drawQueueFail; }

	code = kaelTree_reserve(&book->drawQueue, 8);
	if(code != KAEL_SUCCESS){ goto label_drawQueueFail; }
	
	return code;

	//Failure and free scope
	label_drawQueueFail:
	kaelTree_free(&book->page);

	label_pageFail:
	if(book->rowBuf.ownsBuffer){
		free(book->rowBuf.s);
	}

	label_rowBufFail:

	return code;
}

void kaelBook_freeBook(KaelBook *book){
	while(!kaelTree_empty(&book->page)){
		KaelBook_page *curPage = kaelTree_back(&book->page);
		kaelBook_freePage(curPage);
		kaelTree_pop(&book->page);
	}

	kaelTree_free(&book->page);
	kaelTree_free(&book->drawQueue);

	if(book->rowBuf.ownsBuffer){
		free(book->rowBuf.s);
		book->rowBuf.s=NULL;
	}
	book->rowBuf.readPtr=NULL;
}

void kaelBook_allocPage(KaelBook_page *page){
	kaelTree_alloc(&page->shape, sizeof(KaelBook_shape));
}




//------ Shape col/row visibility conditions  ------

/**
 * @brief is any part of shape between rowY0 and rowY1? (book space)
*/
uint8_t kaelBook_isShapeInRows(KaelBook_shape *shapePtr, uint16_t rowY0, uint16_t rowY1 ){
	KAEL_ASSERT(shapePtr!=NULL);
	uint16_t shapeY0 = shapePtr->pos[1]; //shape corners
	uint16_t shapeY1 = shapePtr->pos[1] + shapePtr->size[1];

	return  	shapeY0 < rowY1 && 	//visible top
				shapeY1 > rowY0;		//visible bottom
}

/**
 * @brief is any part of shape in viewport?
*/
uint8_t kaelBook_isShapeInView(KaelBook *book, KaelBook_shape *shapePtr){
	KAEL_ASSERT(book!=NULL || shapePtr!=NULL);
	uint16_t shapeX0 = shapePtr->pos[0]; //shape corners
	uint16_t shapeY0 = shapePtr->pos[1];
	uint16_t shapeX1 = shapePtr->pos[0] + shapePtr->size[0];
	uint16_t shapeY1 = shapePtr->pos[1] + shapePtr->size[1];

	uint16_t viewX0 = book->viewPos[0]; //view corners
	uint16_t viewY0 = book->viewPos[1];
	uint16_t viewX1 = book->viewPos[0] + book->size[0];
	uint16_t viewY1 = book->viewPos[1] + book->size[1];

	return  	shapeY0 < viewY1 && 	//visible bottom
				shapeY1 > viewY0 && 	//visible top
				shapeX0 < viewX1 &&	//visible right
				shapeX1 > viewX0;	 	//visible left
}

/**
 * @brief is shape char in view width wise? (shape space)
 */
uint8_t kaelBook_isColInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeCol){
	KAEL_ASSERT(book!=NULL || shapePtr!=NULL);
	return 	shapeCol + shapePtr->pos[0] >= book->viewPos[0] 						&&	//visible left
				shapeCol + shapePtr->pos[0]  < book->viewPos[0] + book->size[0];		//visible right
}

/**
 * @brief is shape char in view height wise? (shape space)
 */
uint8_t kaelBook_isRowInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow){
	KAEL_ASSERT(book!=NULL || shapePtr!=NULL);
	return	shapeRow + shapePtr->pos[1] >= book->viewPos[1] 						&&	//visible top
				shapeRow + shapePtr->pos[1]  < book->viewPos[1] + book->size[1];		//visible bottom
}

/**
 * @brief is shape char above top? (shape space)
 */
uint8_t kaelBook_isAboveShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow){
	KAEL_ASSERT(book!=NULL || shapePtr!=NULL);
	return shapeRow + shapePtr->pos[1] < book->viewPos[1];
}

/**
 * @brief is shape char beyond bottom? (shape space)
 */
uint8_t kaelBook_isBelowShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t shapeRow){
	KAEL_ASSERT(book!=NULL || shapePtr!=NULL);
	return shapeRow + shapePtr->pos[1] >= book->viewPos[1] + book->size[1];
}




//--- Arg Inputs are in view space ---

/**
 * @brief What shape is at viewport char col/row? (view space)
 */
KaelBook_shape *kaelBook_getShapeAt(KaelBook *book, uint16_t viewCol, uint16_t viewRow){
	KAEL_ASSERT(book!=NULL);
	uint16_t bookX = book->viewPos[0] + viewCol;
	uint16_t bookY = book->viewPos[1] + viewRow;

	KaelBook_page *pagePtr = kaelTree_get(&book->page, book->index);
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);

	while(shapePtr){
		//shape corners
		uint16_t shapeX0 = shapePtr->pos[0]; 
		uint16_t shapeY0 = shapePtr->pos[1];
		uint16_t shapeX1 = shapePtr->pos[0] + shapePtr->size[0];
		uint16_t shapeY1 = shapePtr->pos[1] + shapePtr->size[1];

		if(bookX >= shapeX0 &&
			bookX  < shapeX1 &&
			bookY >= shapeY0 &&
			bookY  < shapeY1
		 ){
			//Return first instance
			return shapePtr;
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}

	//Returns NULL if no match
	return shapePtr;
}




//------ Shape drawing ------

/**
 * @brief encode drawMode_pixel into byte 
 */
KaelBook_pixel kaelBook_encodePixel(uint8_t color, uint8_t bright, uint8_t length){
	length = length==0 ? 1 : length; //Can't be null byte

	KaelBook_pixel pixel = {0};
	pixel.byte = (color<<5) | (bright&0b1)<<4 | (length&0b1111) ;
	
	return pixel;
}

/**
 * @brief decode byte into drawMode_pixel 
 */
KaelBook_pixel kaelBook_decodePixel(uint8_t byte){
	KaelBook_pixel pixel = {
		.color	= byte>>5,
		.bright	= (byte>>4)&0b1,
		.length	= byte&0b1111
	};
	
	pixel.length 	= pixel.length==0 ? 1 : pixel.length;
	return pixel;
}

/**
 * @brief Convert drawMode_pixel into Krle_ansiStyle
 */
Krle_ansiStyle kaelBook_pixelToStyle(KaelBook_pixel pixel){
	//Pixels have no style, so the last arg can be anything except 0 (reset)
	Krle_ansiStyle ansiCode = krle_encodeStyle( pixel.bright, ANSI_BG, pixel.color, 2); 
	return ansiCode;
}




//------ Book->TUI intermediate functions ------ 

/**
 * @brief Push decoded Text mode byte to string  
 */
void kaelBook_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);

	Krle_ansiStyle ansiStyle = krle_decodeStyle(rawByte);

	if ( ansiStyle.byte == ANSI_RESET ) {
		kaelTui_pushConstChar(rowBuf, KRLE_STYLE_RESET);
		return;
	}

	uint16_t arg1 = krle_attributeTable[ansiStyle.style &0b111 ];
	uint16_t arg2 = krle_colorTable[		ansiStyle.byte &0b11111 ];
 	kaelTui_twoArgEscSeq(rowBuf, arg1, arg2, 'm');
}

/**
 * @brief Write draw-cursor position into buffer in viewport space
 */
void kaelBook_pushMovInShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col, uint16_t row){
	uint16_t movPos[2] = {
		kaelMath_sub(shapePtr->pos[0] + col, book->viewPos[0]), 
		kaelMath_sub(shapePtr->pos[1] + row, book->viewPos[1])
	};
	kaelTui_pushMov(&book->rowBuf, movPos[0], movPos[1]);
}




//------ Panning ------

/**
	@brief Scroll viewport up or down and add affected shapes to queue
*/	
void kaelBook_scrollRows(KaelBook *book, uint16_t scrollCount, uint16_t scrollUp){
	if(scrollCount==0){return;}
	KaelBook_page *pagePtr = kaelTree_get(&book->page, book->index);
	if(kaelTree_empty(&pagePtr->shape)){
		return;
	}

	uint16_t rowY0,rowY1;

	//calculate new row ranges in book space and update viewport position
	if(scrollUp){
		rowY0 = book->viewPos[1] + (book->size[1]); //Beginning of old bottom row
		rowY1 = book->viewPos[1] + (book->size[1]) + scrollCount; //End of new bottom row

		book->viewPos[1]+= scrollCount;
		//No clearing as long viewport stays {0,0}
	}else{
		scrollCount = kaelMath_min(scrollCount, book->viewPos[1]); //Ensure non-negative row
		if(scrollCount==0){
			return;
		}
		
		rowY0 = book->viewPos[1] - scrollCount; //new top row
		rowY1 = book->viewPos[1]; //old top row

		book->viewPos[1]-= scrollCount;
	}
	//Set scroll region
	kaelTui_pushLimitScroll(&book->rowBuf, 0, book->size[1]-1);

	kaelTui_pushScroll(&book->rowBuf, scrollCount, scrollUp);
	//iterator
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		if( kaelBook_isShapeInRows(shapePtr, rowY0, rowY1) ){
			//Check what shapes need to be redrawn
			kaelTree_push(&book->drawQueue, &shapePtr);
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}


/**
	@brief Scroll viewport left or right, requires redrawing everything as most terminal emus don't support this
*/	
void kaelBook_scrollCols(KaelBook *book, uint16_t scrollCount, uint16_t scrollLeft){
	KaelBook_page *pagePtr = kaelTree_get(&book->page, book->index);
	if(kaelTree_empty(&pagePtr->shape)){
		return;
	}

	//Clear terminal
	kaelTui_pushConstChar(&book->rowBuf, KRLE_CLEAR);

	if(scrollLeft){
		book->viewPos[0]+= scrollCount;
	}else{
		scrollCount = kaelMath_min(scrollCount, book->viewPos[0]); //Ensure non-negative row
		book->viewPos[0]-= scrollCount;
	}

	//iterator
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		if( kaelBook_isShapeInView(book, shapePtr) ){
			//Queue every visible shape
			kaelTree_push(&book->drawQueue, &shapePtr);
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}