/**
 * @file bookDraw.c
 * 
 * @brief Implementation, TUI drawing tools parsing KRLE strings
 * 
 */
#include "kaelygon/book/bookDraw.h"


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
			if( (*row >= shapePtr->size[1]) || kaelBook_isBotClip(book, shapePtr, *row) ){
				//Beyond shape or last visible row
				return;
			}
			kaelBook_pushMovInShape(book, shapePtr, *col, *row);
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
	kaelBook_pushMovInShape(book, shapePtr, *col, *row);
}

/**
 * @brief Interpret shape string chars and markers. 
 * 
 */
void kaelBook_parseChar(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row ){
	//Parse character
	switch( (uint8_t)rowBuf->readPtr[0] ){
		case KRLE_TEXT_STYLE:
			//Ansi style and color encoding
			rowBuf->readPtr++; //skip the marker
			kaelBook_pushMarkerStyle(rowBuf, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case KRLE_TEXT_SPACE:
			//Jump by printing white space, necessary for colored BG
			rowBuf->readPtr++; 
			kaelBook_solveSpaceMarker(book, rowBuf, shapePtr, col, row, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case KRLE_TEXT_JUMP:
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
			if(kaelBook_isColInView(book, shapePtr, *col)){
				kaelTui_pushChar(rowBuf, (char*)&rowBuf->readPtr[0], 1);
			}
			//non zero character increments
			(*col)++;
			rowBuf->readPtr++;
			break;
	}
}




//------ Draw different types of data ------

/**
 * @brief Stripes of colored white spaces in row (drawMode_pixel) format: [color : 4bit | length : 4bit] repeating... [NULL]
 */
void kaelBook_drawPixelString(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr){
	uint16_t col=0;
	uint16_t row=0;
	
	while( row < shapePtr->size[1] && ! kaelBook_isBotClip(book, shapePtr, row)){ //within shape
		if(rowBuf->readPtr[0]==0){
			//Null terminate
			break;
		}

		
		switch(rowBuf->readPtr[0]){
			case 0:
				return;
				
			case KRLE_PIXEL_JUMP:
				rowBuf->readPtr++;
				kaelBook_solveJumpMarker(book, shapePtr, &col, &row, rowBuf->readPtr[0]);
				rowBuf->readPtr++;
				break;

			default:
				//pixel
				KaelBook_pixel pixel = kaelBook_decodePixel((uint8_t)rowBuf->readPtr[0]);
				Krle_ansiStyle ansiStyle = kaelBook_pixelToStyle(pixel);
				kaelBook_pushMarkerStyle(rowBuf, ansiStyle.byte);
		
				kaelBook_solveSpaceMarker(book, rowBuf, shapePtr, &col, &row, pixel.length);
				rowBuf->readPtr++;
				break;		
		}

		if(col >= shapePtr->size[0]){
			//Move cursor to next row in shape
			col = 0;
			row++;
			kaelBook_pushMovInShape(book, shapePtr, col, row);
		}
		
	}
}

/**
 * @brief Printing characters and run marker byte instructions (drawMode_default) format: ... [char] [char] ... [marker 8bit] [marker input 8bit] ... [NULL]
 * No multibyte unicode detection as of yet
 */
void kaelBook_drawShapeString(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr){
	uint16_t col=0;
	uint16_t row=0;
	
	while( ! kaelBook_isBotClip(book, shapePtr, row)){ //within shape

		kaelBook_parseChar(book, rowBuf, shapePtr, &col, &row);	

		if(col >= shapePtr->size[0]){
			//Move cursor to next row in shape
			col = 0;
			row++;
			kaelBook_pushMovInShape(book, shapePtr, col, row);
		}
	}
}




//------ High level book manipulation ------

/**
	@brief Place cursor at end of the page and reset style
*/	
void kaelBook_resetStyle(KaelBook *book){
	KaelTui_rowBuffer *rowBuf = &book->rowBuf;
	kaelTui_pushConstChar	(rowBuf, KRLE_SCROLL_RESET);
	kaelTui_pushConstChar	(rowBuf, KRLE_STYLE_RESET);
	kaelTui_pushMov			(rowBuf, 0, book->size[1]+1);
	kaelTui_printRowBuf		(rowBuf);
	fflush(stdout);
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
	kaelBook_pushMovInShape(book, shapePtr, 0, 0);
	
	if(shapePtr->drawMode == drawMode_pixel){
		kaelBook_drawPixelString(book, rowBuf, shapePtr);
	}else{
		kaelBook_drawShapeString(book, rowBuf, shapePtr);
	}

	kaelTui_pushConstChar(rowBuf, KRLE_STYLE_RESET);
}

/**
	@brief print book shape queue
	TODO: Don't draw duplicates
*/	
void kaelBook_drawQueue(KaelBook *book){
	KAEL_ASSERT(book!=NULL);

	//reset screen
	kaelTui_pushConstChar(&book->rowBuf, KRLE_STYLE_RESET);
	
	//Iterate drawQueue
	while(!kaelTree_empty(&book->drawQueue)){
		KaelBook_shape *shapePtr = *(KaelBook_shape **)kaelTree_back(&book->drawQueue);
		kaelBook_drawShape(book, shapePtr);
		kaelTree_pop(&book->drawQueue);
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
		if( kaelBook_isShapeInView(book, shapePtr) ){
			kaelTree_push(&book->drawQueue, &shapePtr);
		}
		kaelTree_next(&pagePtr->shape, (void**)&shapePtr);
	}
}

