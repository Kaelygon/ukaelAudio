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
 * @brief is shape column in view?
 */
uint8_t kaelTui_isColInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col){
	return col + shapePtr->pos[0] >= book->viewPos[0] &&
			 col + shapePtr->pos[0]  < book->viewPos[0] + book->size[0];
}

/**
 * @brief is shape row in view?
 */
uint8_t kaelTui_isRowInView(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] >= book->viewPos[1]
		 && row + shapePtr->pos[1]  < book->viewPos[1] + book->size[1];
}

/**
 * @brief is shape row above top?
 */
uint8_t kaelTui_isTopClip(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] < book->viewPos[1];
}

/**
 * @brief is shape row beyond bottom?
 */
uint8_t kaelTui_isBotClip(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] > book->viewPos[1] + book->size[1];
}




//------ Shape drawing ------


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
		//space remaining in shape
		uint16_t colRemaining = shapePtr->size[0] - *col; 
		uint16_t moveCount = kaelMath_min( spaceCount, colRemaining );
		
		//subtract hidden left columns but account existing chars on row
		uint16_t hiddenLeftCols = kaelMath_sub(book->viewPos[0], shapePtr->pos[0]);
		uint16_t visibleSpace = moveCount - kaelMath_sub(hiddenLeftCols, *col); 

		//space remaining in viewPort
		uint16_t charCol = shapePtr->pos[0] + *col;
		uint16_t viewEndCol = book->size[0] + book->viewPos[0];
		uint16_t viewRemaining = kaelMath_sub(viewEndCol, charCol);
		visibleSpace = kaelMath_min( viewRemaining, visibleSpace );

		//print visible white spaces
		kaelTui_pushSpace(rowBuf, visibleSpace); 

		//virtually tracked spaces
		spaceCount -= moveCount; 
		*col += moveCount;

		//next row
		if(*col >= shapePtr->size[0]){
			*col=0;
			(*row)++;
			//beyond last row
			if( kaelTui_isBotClip(book, shapePtr, *row) ){
				break;
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
			//Check if column is visible and top is not clipped 
			if(kaelTui_isColInView(book, shapePtr, *col) && ! kaelTui_isTopClip(book, shapePtr, *row) ){
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
	uint16_t col=0;
	uint16_t row=0;
	kaelBook_movInShape(book, shapePtr, col, row);
	
	while(row < shapePtr->size[1]){ //within shape
		if(col >= shapePtr->size[0]){
			//Move cursor to next row in shape
			col=0;
			row++;
			kaelBook_movInShape(book, shapePtr, col, row);
		}

		if( kaelTui_isBotClip(book, shapePtr, row) ){
			//Bottom rows can't affect view
			return;
		}

		kaelBook_parseChar(book, rowBuf, shapePtr, &col, &row);	
	}
	return;
}

/**
	@brief print book shape queue
*/	
void kaelBook_drawQueue(KaelBook *book){
	KAEL_ASSERT(book!=NULL);

	//reset screen
	const char *termRst = "\033[2J";
	kaelTui_pushMarkerStyle(&book->rowBuf, ansiReset);
	kaelTui_pushChar(&book->rowBuf,termRst,sizeof(termRst));
	
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
	KaelBook_page *pagePtr = kaelTree_get(&book->page,index);

	//Iterate shapes
	if(kaelTree_empty(&pagePtr->shape)){
		return;
	}
	KaelBook_shape *shapePtr = kaelTree_begin(&pagePtr->shape);
	while(shapePtr){
		//TODO: check if shape overlaps with viewport and push only those
		kaelTree_push(&book->drawQueue, &shapePtr);
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}

/**
	@brief Place cursor at end of the page and reset style
*/	
void kaelBook_resetStyle(KaelBook *book){
	kaelTui_pushMarkerStyle(&book->rowBuf, ansiReset);
	kaelTui_pushMov(&book->rowBuf, 0, book->size[1]);
	kaelTui_printRowBuf(&book->rowBuf);
}
