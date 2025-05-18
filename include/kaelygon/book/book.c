/**
 * @file book.c
 * 
 * @brief Implementation, Print TUI pages
 * 
 * Why this amalgamation? Because I decided our print buffer is 256 bytes for no particular reason other than as a challenge
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
 * @brief Encode ansi color into a single byte
 */
char kaelTui_decodeAnsiEsc(const uint8_t mod, const uint8_t color, const uint8_t style){
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
uint8_t kaelTui_encodeAnsiEsc(uint8_t *escSeq, const uint8_t ansiByte, uint16_t offset){
	KAEL_ASSERT(escSeq!=NULL);

	if ( ansiByte == ansiReset ) {
		escSeq[offset+0]='\x1b';
		escSeq[offset+1]='[';
		escSeq[offset+2]='0';
		escSeq[offset+3]='m';
		return 4;
	}

	KaelTui_ansiCode code = {.byte=ansiByte};
	escSeq[offset+0]='\x1b';
	escSeq[offset+1]='[';
	escSeq[offset+2]=code.style+'0';
	escSeq[offset+3]=';';
	escSeq[offset+4]=AnsiModValue[code.mod]+'0';
	escSeq[offset+5]=code.color+'0';
	escSeq[offset+6]='m';
	return 7;
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

//------ Print shape ------

void kaelTui_freePage(KaelBook_page *page){
	while(!kaelTree_empty(&page->shape)){
		kaelTree_pop(&page->shape);
	}
	kaelTree_free(&page->shape);
}


/**
 * @brief Check if rowBuf has at least bytes free space
 */
uint8_t kaelTui_fitsInBuf(KaelTui_RowBuffer *rowBuf, uint16_t bytes){
	KAEL_ASSERT(rowBuf->pos <= rowBuf->size, "rowBuf overflow");
	return (rowBuf->pos + bytes <= rowBuf->size);
}

void kaelTui_printRowBuf(KaelTui_RowBuffer *rowBuf){
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
void kaelTui_printFullBuf(KaelTui_RowBuffer *rowBuf, uint16_t bytes){
	if( !kaelTui_fitsInBuf(rowBuf,bytes) ){
		kaelTui_printRowBuf(rowBuf);
	}
}

/**
 * @brief Print white spaces
*/
void kaelTui_printeSpace(KaelTui_RowBuffer *rowBuf, uint16_t spaceCount){
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

/**
 * @brief Push string to rowbuf
*/
void kaelTui_pushChar(KaelTui_RowBuffer *rowBuf, const char *string, const uint8_t bytes){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(string!=NULL);

	kaelTui_printFullBuf(rowBuf,bytes);
	memcpy((uint8_t*)&rowBuf->s[rowBuf->pos], string, sizeof(uint8_t)*bytes);
	rowBuf->pos+=bytes;
	return;
}

void kaelTui_printMarkerStyle(KaelTui_RowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);
	if(rawByte==0){
		return; //NULL data
	};
	//Check if rowBuf has enough space for ansiCode
	kaelTui_printFullBuf(rowBuf, ansiLength);
	rowBuf->pos+=kaelTui_encodeAnsiEsc(rowBuf->s, rawByte, rowBuf->pos);
	return;
}



//fetch page shapes that overlap with row
void kaelTui_getRowShapes( const KaelBook_page *page, KaelTree *shapePtrList, uint16_t row ){
	KaelBook_shape *shapePtr;

	for(uint16_t i=0; i < kaelTree_length(&page->shape); i++){ 
		shapePtr = kaelTree_get(&page->shape, i);

		uint16_t shapeStartRow = shapePtr->pos[1];
		uint16_t shapeEndRow   = shapePtr->pos[1] + shapePtr->size[1];
		uint8_t condition = row >= shapeStartRow && row < shapeEndRow;  //If shape overlaps with row

		if( condition ){
			kaelTree_push(shapePtrList, &shapePtr);
		}
	}
}

void kaelTui_parseChar( KaelBook *book, KaelTui_RowBuffer *rowBuf, KaelBook_shape *shapePtr ){
	//Parse character
	switch( (uint8_t)rowBuf->readPtr[0] ){
		case markerJump:
			//Backlog jumps for next row
			rowBuf->readPtr++; //skip the marker
			shapePtr->jumpsRemaining = rowBuf->readPtr[0]; 
			rowBuf->readPtr++;
			break;

		case markerStyle:
			//Ansi style and color encoding
			rowBuf->readPtr++;
			shapePtr->lastStyle = rowBuf->readPtr[0];
			kaelTui_printMarkerStyle(rowBuf, rowBuf->readPtr[0]);
			rowBuf->readPtr++;
			break;

		case 0: 
			//NULL termination
			book->pos[0]++;
			break;

		default: 
			kaelTui_pushChar(rowBuf, (char*)&rowBuf->readPtr[0], 1);
			//non zero character increments
			book->pos[0]++;
			rowBuf->readPtr++;
			break;
	}
}


/**
 * @brief print current page row 
 */
void kaelTui_printRow( KaelBook *book, KaelBook_page *page ){

	KaelTui_RowBuffer *rowBuf = book->rowBuf;
	KaelTree *shapePtrList = &book->shapePtrList;

	//Reset and populate shapePtr for this row
	kaelTree_resize(shapePtrList, 0);
	kaelTui_getRowShapes( page, shapePtrList, book->pos[1]);

	//shapePtrList Iterator
	KaelBook_shape *shapePtr = NULL;
	uint16_t shapePtrIndex = 0;
	uint16_t shapePtrCount = kaelTree_length(shapePtrList);

	//shapePtr string start
	uint8_t *readStart=NULL;

	while( !kaelTree_empty(shapePtrList) ){

		//next shape
		if( shapePtr==NULL || (book->pos[0] - shapePtr->pos[0]) >= shapePtr->size[0] ){ //shape is NULL OR book->pos[0] exceeds shape 

			if( shapePtrIndex>=shapePtrCount ){ //if current==end
				break; 
			}

			kaelTui_storeReadCount(shapePtr, readStart, rowBuf->readPtr);
			
			//next shape pointer
			shapePtr = *(KaelBook_shape **)kaelTree_get(shapePtrList, shapePtrIndex);
			shapePtrIndex++;
			if(shapePtr==NULL){
				break;
			}

			//continue reading shape string at shape readhead
			rowBuf->readPtr = (uint8_t *)shapePtr->string + shapePtr->readHead;
			readStart = (uint8_t *)rowBuf->readPtr;

			//jump to next shape
			if( book->pos[0] < shapePtr->pos[0] ){
				uint16_t jumpCount = shapePtr->pos[0] - book->pos[0];
				kaelTui_printMarkerStyle(rowBuf, ansiReset);
				kaelTui_printeSpace(rowBuf, jumpCount);
				kaelTui_printMarkerStyle(rowBuf, shapePtr->lastStyle); 

				book->pos[0]+=jumpCount;
			}

		}

		//if past last column
		if(book->pos[0] >= book->size[0]){
			break;
		}

		//Check if shapePtr has backlogged jumps
		if( shapePtr->jumpsRemaining ){
			kaelTui_printMarkerStyle(rowBuf, shapePtr->lastStyle);
			uint8_t shapeColsRemaining = shapePtr->size[0] - (book->pos[0] - shapePtr->pos[0]);
			uint8_t jumpCount = kaelMath_min( shapePtr->jumpsRemaining, shapeColsRemaining ); //Can't exceed shape
			kaelTui_printeSpace(rowBuf, jumpCount);
			shapePtr->jumpsRemaining-=jumpCount;
			book->pos[0]+=jumpCount;
			continue;
		}

		kaelTui_parseChar(book, rowBuf, shapePtr);
	}

	kaelTui_storeReadCount(shapePtr, readStart, rowBuf->readPtr);
			
	//Start new line
	kaelTui_printMarkerStyle(rowBuf, ansiReset);
	kaelTui_pushChar(rowBuf, "\n", 1);
	book->pos[0]=0;
}



/**
	@brief print page shapes in viewport row by row

	Private Use Area (PUA), 2-byte range: U+E000 to U+F8FF
	Linux ANSI color escape sequences are atrociously long, up to 8 bytes '\x1b'[1;37m
	Let us use unicode PUA BMP range U+E0_00 to U+F8_FF
	
	Check if the first byte falls in this range then it is a marker
	otherwise print unicode by the given length 'L', 
	number of leading 1s set the unicode length, [0b1L...0#] [0b10######] [0b10######] ...
	e.g. 0b11110000 has total 4 bytes, the trailing bits are part of the unicode
	
	Since 0xE0 to 0xF8 are 
*/	
void kaelTui_printPage( KaelBook *book){
	if(kaelTree_empty(&book->page)){
		return;
	}

	//Reset to top 
	const char *ansiMovTop = "\x1b[0";
	kaelTui_pushChar(book->rowBuf, ansiMovTop, strlen(ansiMovTop));

	KaelBook_page *curPage = kaelTree_get(&book->page,book->index);

	//Iterate viewport rows
	for(book->pos[1]=0; book->pos[1] < book->size[1]; book->pos[1]++){
		kaelTui_printRow( book, curPage );
	}
	//Print remaining buffer if any
	kaelTui_printRowBuf(book->rowBuf);

	//Reset readheads
	for(uint16_t i=0; i < kaelTree_length(&curPage->shape); i++){ 
		KaelBook_shape *shapePtr = kaelTree_get(&curPage->shape, i);
		shapePtr->readHead = 0;
	}

	//page ends, flush viewport
	fflush(stdout);
}

