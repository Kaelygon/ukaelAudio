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

uint8_t kaelBook_allocBook(KaelBook *book, const uint16_t viewWidth, const uint16_t viewHeight, const uint16_t printBufferSize){

	*book = (KaelBook){0};
	book->size[0]    = viewWidth;
	book->size[1]    = viewHeight;

	book->rowBuf = (KaelTui_rowBuffer){0};
	book->rowBuf.size = printBufferSize;
	book->rowBuf.s = calloc(book->rowBuf.size, sizeof(uint8_t));

	if(NULL_CHECK(book->rowBuf.s)){
		return KAEL_ERR_NULL;
	}

	uint8_t code = KAEL_SUCCESS;

	code = kaelTree_alloc(&book->page, sizeof(KaelBook_page));
	if(code != KAEL_SUCCESS){
		free(book->rowBuf.s);
		return code;
	}

	code = kaelTree_alloc(&book->drawQueue, sizeof(KaelBook_shape));
	if(code != KAEL_SUCCESS){
		free(book->rowBuf.s);
		kaelTree_free(&book->page);
		return code;
	}

	kaelTree_reserve(&book->drawQueue, 8);

	return KAEL_SUCCESS;
}

void kaelBook_freeBook(KaelBook *book){
	while(!kaelTree_empty(&book->page)){
		KaelBook_page *curPage = kaelTree_back(&book->page);
		kaelBook_freePage(curPage);
		kaelTree_pop(&book->page);
	}

	kaelTree_free(&book->page);
	kaelTree_free(&book->drawQueue);

	free(book->rowBuf.s);
	book->rowBuf.readPtr=NULL;
	book->rowBuf.s=NULL;
}

void kaelBook_allocPage(KaelBook_page *page){
	kaelTree_alloc(&page->shape, sizeof(KaelBook_shape));
}



//------ Shape col/row conditions  ------

/**
 * @brief is any part of shape between rowY0 and rowY1?
*/
uint8_t kaelTui_isShapeInRows(KaelBook_shape *shapePtr, uint16_t rowY0, uint16_t rowY1 ){
	uint16_t shapeY0 = shapePtr->pos[1]; //shape corners
	uint16_t shapeY1 = shapePtr->pos[1] + shapePtr->size[1];

	return  	shapeY0 < rowY1 && 	//visible top
				shapeY1 > rowY0;		//visible bottom
}

/**
 * @brief is any part of shape in viewport?
*/
uint8_t kaelTui_isShapeInView(KaelBook *book, KaelBook_shape *shapePtr){
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
 * @brief is shape char in view width wise?
 */
uint8_t kaelTui_isColInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col){
	return 	col + shapePtr->pos[0] >= book->viewPos[0] 						&&	//visible left
			 	col + shapePtr->pos[0]  < book->viewPos[0] + book->size[0];		//visible right
}

/**
 * @brief is shape char in view height wise?
 */
uint8_t kaelTui_isRowInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return	row + shapePtr->pos[1] >= book->viewPos[1] 						&&	//visible top
				row + shapePtr->pos[1]  < book->viewPos[1] + book->size[1];		//visible bottom
}

/**
 * @brief is shape char above top?
 */
uint8_t kaelTui_isTopClip(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] < book->viewPos[1];
}

/**
 * @brief is shape char beyond bottom?
 */
uint8_t kaelTui_isBotClip(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] >= book->viewPos[1] + book->size[1];
}




//------ Shape drawing ------

KaelBook_pixel kaelBook_encodePixel(uint8_t bright, uint8_t color, uint8_t length){
	KaelBook_pixel pixel;
	pixel.bright = bright&0b1;
	pixel.color = color&0b111;
	pixel.length = length&0b1111;
	pixel.length = pixel.length==0 ? 1 : pixel.length;
	return pixel;
}

KaelBook_pixel kaelBook_decodePixel(uint8_t byte){
	KaelBook_pixel pixel;
	pixel.byte = byte;
	pixel.length = pixel.length==0 ? 1 : pixel.length;
	return pixel;
}

KaelTui_ansiStyle kaelBook_pixelToStyle(KaelBook_pixel pixel){
	uint8_t ansiMod = pixel.bright ? ansiBGHigh : ansiBGLow;
	KaelTui_ansiStyle ansiCode = kaelTui_encodeStyle(ansiMod, pixel.color, 0U);
	return ansiCode;
}


/**
 * @brief Write cursor position into buffer in viewport space
 */
void kaelBook_movInShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col, uint16_t row){
	uint16_t movPos[2] = {
		kaelMath_sub(shapePtr->pos[0] + col, book->viewPos[0]), 
		kaelMath_sub(shapePtr->pos[1] + row, book->viewPos[1])
	};
	kaelTui_pushMov(&book->rowBuf, movPos[0], movPos[1]);
}

/**
 * @brief Move in shape printing white space
 * 
 * Necessary for colored overdraw. Compute what parts of the space are clipped (left, top or right) and add white space to buffer
 * 
 */
void kaelBook_solveSpaceMarker(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row, uint8_t spaceCount){
	while( spaceCount > 0 ){
		//How many columns are on right?
		uint16_t colRemaining = shapePtr->size[0] - *col; 
		uint16_t moveCount = kaelMath_min( spaceCount, colRemaining );
		
		//How many columns are hidden on left?
		uint16_t hiddenLeftCols = kaelMath_sub(book->viewPos[0], shapePtr->pos[0]);
					hiddenLeftCols = kaelMath_sub(hiddenLeftCols, *col); //account current col position
		uint16_t visibleSpace  = kaelMath_sub(moveCount, hiddenLeftCols); //spaces in viewport

		//Space remaining in viewPort
		uint16_t charCol = shapePtr->pos[0] + *col;
		uint16_t viewEndCol = book->size[0] + book->viewPos[0];
		uint16_t viewRemaining = kaelMath_sub(viewEndCol, charCol);
		visibleSpace = kaelMath_min( viewRemaining, visibleSpace );

		//print visible white spaces
		kaelTui_pushSpace(rowBuf, visibleSpace); 

		//Book-space tracked white spaces
		spaceCount -= moveCount; 
		*col += moveCount;

		if(*col >= shapePtr->size[0]){
			//Next row
			*col=0;
			(*row)++;
			if( (*row >= shapePtr->size[1]) || kaelTui_isBotClip(book, shapePtr, *row) ){
				//Beyond shape or last visible row
				return;
			}
			kaelBook_movInShape(book, shapePtr, *col, *row);
		}
	}
}

/**
 * @brief Move in shape without overdraw
 */
void kaelBook_solveJumpMarker(KaelBook *book, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row, uint8_t jumpCount){
	uint16_t jumpCols = jumpCount % shapePtr->size[0];
	uint16_t jumpRows = jumpCount / shapePtr->size[0];
	*col+=jumpCols;
	*row+=jumpRows;
	kaelBook_movInShape(book, shapePtr, *col, *row);
}

/**
 * @brief Interpret shape string chars and markers. 
 * 
 */
void kaelBook_parseChar(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row ){
	//Parse character
	switch( (uint8_t)rowBuf->readPtr[0] ){
		case markerStyle:
			//Ansi style and color encoding
			rowBuf->readPtr++; //skip the marker
			kaelTui_pushMarkerStyle(rowBuf, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case markerSpace:
			//Jump by printing white space, necessary for colored BG
			rowBuf->readPtr++; 
			kaelBook_solveSpaceMarker(book, rowBuf, shapePtr, col, row, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case markerJump:
			//Jump without overdraw
			rowBuf->readPtr++;
			kaelBook_solveJumpMarker(book, shapePtr, col, row, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case 0: 
			//NULL termination
			(*col)++;
			break;

		default: 
			//Check if column is visible
			if(kaelTui_isColInView(book, shapePtr, *col)){
				kaelTui_pushChar(rowBuf, (char*)&rowBuf->readPtr[0], 1);
			}
			//non zero character increments
			(*col)++;
			rowBuf->readPtr++;
			break;
	}
}

//------ High level book manipulation ------

/**
	@brief Place cursor at end of the page and reset style
*/	
void kaelBook_resetStyle(KaelBook *book){
	kaelTui_pushMarkerStyle(&book->rowBuf, ansiReset);
	kaelTui_pushMov(&book->rowBuf, 0, book->size[1]+1);
	kaelTui_printRowBuf(&book->rowBuf);
	fflush(stdout);
}

void kaelBook_drawPixelString(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr){
	uint16_t col=0;
	uint16_t row=0;
	while( row < shapePtr->size[1] && ! kaelTui_isBotClip(book, shapePtr, row)){ //within shape
		if(rowBuf->readPtr[0]==0){
			//Null terminate
			break;
		}

		kaelBook_movInShape(book, shapePtr, col, row);

		KaelBook_pixel pixel = kaelBook_decodePixel((uint8_t)rowBuf->readPtr[0]);
		rowBuf->readPtr++;
		//Get color
		KaelTui_ansiStyle ansiStyle = kaelBook_pixelToStyle(pixel);
		kaelTui_pushMarkerStyle(rowBuf, ansiStyle.byte);

		kaelBook_solveSpaceMarker(book, rowBuf, shapePtr, &col, &row, pixel.length);

		if(col >= shapePtr->size[0]){
			//Move cursor to next row in shape
			col = 0;
			row++;
		}
	}
}

void kaelBook_drawShapeString(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr){
	uint16_t col=0;
	uint16_t row=0;
	while( ! kaelTui_isBotClip(book, shapePtr, row)){ //within shape
		kaelBook_movInShape(book, shapePtr, col, row);

		kaelBook_parseChar(book, rowBuf, shapePtr, &col, &row);	

		if(col >= shapePtr->size[0]){
			//Move cursor to next row in shape
			col = 0;
			row++;
		}
	}

}

/**
	@brief Print shape string in viewport space. Markers  
 * 
 * @note Markers even outside of view must be read as they can effect viewport
 * 
*/	
void kaelBook_drawShape(KaelBook *book, KaelBook_shape *shapePtr){
	kaelTui_pushMov(&book->rowBuf, shapePtr->pos[0], shapePtr->pos[1]);
	KaelTui_rowBuffer *rowBuf = &book->rowBuf;
	rowBuf->readPtr = (uint8_t *)shapePtr->string;

	//shape space position
	kaelBook_movInShape(book, shapePtr, 0, 0);
	
	if(shapePtr->drawMode == drawMode_pixel){
		kaelBook_drawPixelString(book, rowBuf, shapePtr);
	}else{
		kaelBook_drawShapeString(book, rowBuf, shapePtr);
	}

	kaelTui_pushMarkerStyle(rowBuf, ansiReset);
}

/**
	@brief print book shape queue
*/	
void kaelBook_drawQueue(KaelBook *book){
	KAEL_ASSERT(book!=NULL);

	//reset screen
	kaelTui_pushMarkerStyle(&book->rowBuf, ansiReset);
	
	//Iterate drawQueue
	while(!kaelTree_empty(&book->drawQueue)){
		KaelBook_shape *shapePtr = *(KaelBook_shape **)kaelTree_back(&book->drawQueue);
		kaelTree_pop(&book->drawQueue);

		kaelBook_drawShape(book, shapePtr);
	}

	//reset and mov cursor to end
	kaelTui_pushMov(&book->rowBuf, 0, book->size[1]);
	kaelTui_printRowBuf(&book->rowBuf);
	fflush(stdout);
}
/**
 * @brief Switch active page and add every shape to draw queue that are visible
 */
void kaelBook_switchPage(KaelBook *book, uint16_t index){
	KAEL_ASSERT(book!=NULL);

	if(kaelTree_empty(&book->page)){
		return;
	}
	index = kaelMath_min(index, kaelTree_length(&book->page));
	book->index=index;
}

/**
 * @brief add page shapes visible in viewport to drawQueue 
 */
void kaelBook_queueViewShapes(KaelBook *book){
	KaelBook_page *pagePtr = kaelTree_get(&book->page, book->index);
	if(kaelTree_empty(&pagePtr->shape)){
		return;
	}
	//iterator
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		if( kaelTui_isShapeInView(book, shapePtr) ){
			kaelTree_push(&book->drawQueue, &shapePtr);
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}

/**
	@brief Scroll viewport up or down and add affected shapes to queue
*/	
void kaelBook_scrollRows(KaelBook *book, uint16_t scrollCount, uint16_t scrollUp){
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

		//Clear bottom rows that will go out of viewport
		uint16_t firstClippedRow = book->size[1] -scrollCount;
		for(uint16_t i=0; i<scrollCount; i++){
			kaelTui_pushMov(&book->rowBuf, 0, firstClippedRow + i);
			kaelTui_pushChar(&book->rowBuf, kaelTui_escSeq[escSeq_clearRow], kaelTui_escSeqLen[escSeq_clearRow]);
		}
	}

	kaelTui_pushScroll(&book->rowBuf, scrollCount, scrollUp);
	//iterator
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		if( kaelTui_isShapeInRows(shapePtr, rowY0, rowY1) ){
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
	kaelTui_pushChar(&book->rowBuf, kaelTui_escSeq[escSeq_clear], kaelTui_escSeqLen[escSeq_clear]);

	if(scrollLeft){
		book->viewPos[0]+= scrollCount;
	}else{
		scrollCount = kaelMath_min(scrollCount, book->viewPos[0]); //Ensure non-negative row
		book->viewPos[0]-= scrollCount;
	}

	//iterator
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		if( kaelTui_isShapeInView(book, shapePtr) ){
			//Queue every visible shape
			kaelTree_push(&book->drawQueue, &shapePtr);
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}