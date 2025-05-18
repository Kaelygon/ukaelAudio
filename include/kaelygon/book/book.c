/**
 * @file book.c
 * 
 * @brief Implementation, Print TUI pages
 * 
 * Hierarchy
 * book -> page -> shape -> string
 */

#include "kaelygon/book/book.h"

/**
 * @brief Offsets to get fore-/background colors low and high
 * tens place. 3*10 + ansiBlue = ansiFGLow blue
 */
uint8_t AnsiModValue[4] = { 
	3, //ansiFGLow
	4, //ansiFGHigh
	7, //ansiBGLow
	9, //ansiBGHigh
};





/**
 * @brief free page trees
 */
void kaelTui_freePage(KaelBook_page *page){
	while(!kaelTree_empty(&page->shape)){
		kaelTree_pop(&page->shape);
	}
	kaelTree_free(&page->shape);
}

/**
 * @brief Encode ansi color into a single byte
 */
char kaelTui_decodeStyle(const uint8_t mod, const uint8_t color, const uint8_t style){
	KaelTui_ansiCode code = {
		.mod = mod,
		.color = color,
		.style = style
	};
	return code.byte;
}

/**
 * @brief decode ansi color escape sequence e.g \x1b[1;37m from KaelTui_ansiCode.byte
 * 
 * @warning No NULL termination  
 * 
 * example: \esc[underline];[ansiBGLow][blue]m = "\esc4;74m"
 * @return number of bytes written
 */
uint8_t kaelTui_encodeStyle(uint8_t *escSeq, const uint8_t ansiByte, uint16_t offset){
	KAEL_ASSERT(escSeq!=NULL);

	if (ansiByte == ansiReset) {
		return snprintf((char *)(escSeq + offset), 5, "\x1b[0m");
	}

	KaelTui_ansiCode code = {.byte = ansiByte};
	return snprintf((char *)(escSeq + offset), 8, "\x1b[%u;%u%um", code.style, AnsiModValue[code.mod], code.color);
}

/**
 * @brief Calculate how much of the shape string was read, and store the progress in shapePtr 
 */
void kaelTui_storeReadCount(KaelBook_shape *shapePtr, const uint8_t* startPtr, const uint8_t* endPtr ){
	if(shapePtr!=NULL){
		uint16_t readCount = endPtr - startPtr;
		shapePtr->readHead += readCount;
	}
}




//------ Buffer printing ------

/**
 * @brief Check if rowBuf has at least bytes free space
 */
uint8_t kaelTui_fitsInBuf(KaelTui_rowBuffer *rowBuf, uint16_t bytes){
	KAEL_ASSERT(rowBuf->pos <= rowBuf->size, "rowBuf overflow");
	return (rowBuf->pos + bytes <= rowBuf->size);
}

void kaelTui_printRowBuf(KaelTui_rowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	//read head can't be beyond its size
	KAEL_ASSERT(rowBuf->pos <= rowBuf->size, "rowBuf overflow");
	//Print only up to overwritten part
	fwrite((char *)rowBuf->s, sizeof(char), rowBuf->pos, stdout); 
	rowBuf->pos=0;
	return;
}

/**
 * @brief Print rowBuf if bytes won't fit
 */
void kaelTui_printFullBuf(KaelTui_rowBuffer *rowBuf, uint16_t bytes){
	if( !kaelTui_fitsInBuf(rowBuf,bytes) ){
		kaelTui_printRowBuf(rowBuf);
	}
}




//------ Push to buffer  ------

/**
 * @brief Print white spaces
*/
void kaelTui_pushSpace(KaelTui_rowBuffer *rowBuf, uint16_t spaceCount){
	KAEL_ASSERT(rowBuf!=NULL);
	if(spaceCount==0){
		return;  //NULL data
	};
	uint8_t batches = spaceCount/rowBuf->size;
	uint8_t len    = spaceCount - batches * rowBuf->size;

	kaelTui_printFullBuf(rowBuf,len);
	memset((uint8_t*)&rowBuf->s[rowBuf->pos], ' ', sizeof(uint8_t)*len);
	rowBuf->pos+=len;

	//if spaceCount is larger than rowBuf->size
	for(uint16_t i=0; i<batches; i++){
		kaelTui_printRowBuf(rowBuf);
		memset(rowBuf->s, ' ', rowBuf->size);
		fwrite(rowBuf->s, sizeof(char), rowBuf->size, stdout);
	}
}

void kaelTui_pushMarkerStyle(KaelTui_rowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);
	if(rawByte==0){
		return; //NULL data
	};
	//Check if rowBuf has enough space for ansiCode
	kaelTui_printFullBuf(rowBuf, ansiLength);
	rowBuf->pos+=kaelTui_encodeStyle(rowBuf->s, rawByte, rowBuf->pos);
	return;
}

/**
 * @brief Push bytes of string to rowbuf
*/
void kaelTui_pushChar(KaelTui_rowBuffer *rowBuf, const char *string, const uint8_t bytes){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(string!=NULL);

	kaelTui_printFullBuf(rowBuf,bytes);
	memcpy((uint8_t*)&rowBuf->s[rowBuf->pos], string, sizeof(uint8_t)*bytes);
	rowBuf->pos+=bytes;
	return;
}

/**
 * @brief Push escSeq Move terminal cursor
*/
void kaelTui_pushMov(KaelTui_rowBuffer *rowBuf, uint16_t col, uint16_t row){
	char escSeq[16]; //"\033[%u;%u"
	uint8_t len = snprintf(escSeq, sizeof(escSeq), "\033[%u;%uH", row+1, col+1);
	kaelTui_pushChar(rowBuf, escSeq, len);
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
 * @brief is past shape last row?
 */
uint8_t kaelTui_isPastLastRow(KaelBook *book, KaelBook_shape *shapePtr, uint16_t row){
	return row + shapePtr->pos[1] > book->viewPos[1] + book->size[1];
}




//------ Shape drawing ------

/**
 * @brief Switch active page and add every shape to draw queue that are visible
 */
void kaelTui_switchPage(KaelBook *book, uint16_t index){
	if(kaelTree_empty(&book->page)){
		return;
	}
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
 * @brief Write cursor position into buffer in viewport space
 */
void kaelTui_movInShape(KaelBook *book, KaelBook_shape *shapePtr, uint16_t col, uint16_t row){
	uint16_t movPos[2] = {
		shapePtr->pos[0] + col - book->viewPos[0], 
		shapePtr->pos[1] + row - book->viewPos[1]
	};
	kaelTui_pushMov(book->rowBuf, movPos[0], movPos[1]);
}

/**
 * @brief Compute what parts of the jump are clipped (left, top or right) and add white space to buffer
 */
void kaelTui_solveJumpMarker(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row, uint8_t jumpCount){
	while( jumpCount > 0 ){
		//space remaining in shape
		uint16_t colRemaining = shapePtr->size[0] - *col; 
		uint16_t moveCount = kaelMath_min( jumpCount, colRemaining );
		
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
		jumpCount -= moveCount; 
		*col += moveCount;

		//next row
		if(*col >= shapePtr->size[0]){
			*col=0;
			(*row)++;
			//beyond last row
			uint8_t rowPastView = *row + shapePtr->pos[1] > book->viewPos[1] + book->size[1];
			if( rowPastView ){
				break;
			}
			kaelTui_movInShape(book, shapePtr, *col, *row);
		}
	}
}

/**
 * @brief Interpret shape string chars and markers
 */
void kaelTui_parseChar(KaelBook *book, KaelTui_rowBuffer *rowBuf, KaelBook_shape *shapePtr, uint16_t *col, uint16_t *row ){
	//Parse character
	switch( (uint8_t)rowBuf->readPtr[0] ){
		case markerJump:
			//Jump by printing white space, necessary for colored BG
			rowBuf->readPtr++; //skip the marker
			uint8_t jumpCount = rowBuf->readPtr[0];
			kaelTui_solveJumpMarker(book, rowBuf, shapePtr, col, row, jumpCount);
			rowBuf->readPtr++;
			break;

		case markerStyle:
			//Ansi style and color encoding
			rowBuf->readPtr++;
			kaelTui_pushMarkerStyle(rowBuf, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case 0: 
			//NULL termination
			(*col)++;
			break;

		default: 
			if(kaelTui_isColInView(book, shapePtr, *col)){
				kaelTui_pushChar(rowBuf, (char*)&rowBuf->readPtr[0], 1);
			}
			//non zero character increments
			(*col)++;
			rowBuf->readPtr++;
			break;
	}
}

void kaelTui_drawShape(KaelBook *book, KaelBook_shape *shapePtr){
	kaelTui_pushMov(book->rowBuf, shapePtr->pos[0], shapePtr->pos[1]);
	KaelTui_rowBuffer *rowBuf = book->rowBuf;
	rowBuf->readPtr = (uint8_t *)shapePtr->string;

	//shape space position
	uint16_t col=0;
	uint16_t row=0;
	kaelTui_movInShape(book, shapePtr, col, row);
	
	while(row < shapePtr->size[1]){ //within shape
		if(col >= shapePtr->size[0]){
			//Move cursor to next line in shape
			col=0;
			row++;
			kaelTui_movInShape(book, shapePtr, col, row);
		}

		//We need to obtain markers even if they are outside viewport (left or above)
		
		if( kaelTui_isPastLastRow(book, shapePtr, row) ){
			return;
		}

		kaelTui_parseChar(book, rowBuf, shapePtr, &col, &row);	
	}
	return;
}

/**
	@brief print book shape queue
*/	
void kaelTui_drawQueue(KaelBook *book){
	//reset screen
	const char *termRst = "\033[2J";
	kaelTui_pushMarkerStyle(book->rowBuf, ansiReset);
	kaelTui_pushChar(book->rowBuf,termRst,sizeof(termRst));
	
	//Iterate and clear queue
	while(!kaelTree_empty(&book->drawQueue)){
		KaelBook_shape *shapePtr = *(KaelBook_shape **)kaelTree_back(&book->drawQueue);
		kaelTui_drawShape(book, shapePtr);
		kaelTree_pop(&book->drawQueue);
	}

	//reset and mov cursor to end
	kaelTui_pushMov(book->rowBuf, book->size[0], book->size[1]);
	kaelTui_printRowBuf(book->rowBuf);
	fflush(stdout);
}
