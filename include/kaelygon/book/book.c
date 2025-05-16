/**
 * @file book.c
 * 
 * @brief Implementation, Print TUI pages
 * 
 * Why this amalgamation? Because I decided our print buffer is 256 bytes for no particular reason other than as a challenge
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
	kaelTui_fitsInBuf(rowBuf,0);
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
	KAEL_ASSERT(rowBuf!=0);
	if(spaceCount==0){
		return;  //NULL data
	};

	for(uint16_t i=0; i<spaceCount; i++){ //jump
		kaelTui_printFullBuf(rowBuf,1);
		rowBuf->s[rowBuf->pos]=' ';
		rowBuf->pos+=1;
	}
	return;
}

void kaelTui_printMarkerStyle(KaelTui_RowBuffer *rowBuf, uint8_t rawByte){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(rowBuf!=0);
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
		uint8_t condition = row >= shapeStartRow && row < shapeEndRow; 

		if( condition ){ //If shape overlaps with row
			kaelTree_push(shapePtrList, &shapePtr);
		}
	}
}

//print shape row by row
void kaelTui_printRow( const KaelBook_page *page, KaelTui_RowBuffer *rowBuf, KaelTree *shapePtrList,  uint16_t currentCol, uint16_t currentRow ){
	//Reset shape list for each row
	kaelTree_resize(shapePtrList, 0);

	uint16_t shapePtrIndex=0;
	uint16_t shapePtrCount=0;
	//rowBuf->readPtr will be incremented so we have to keep track of where we started in shaePtr->string
	const uint8_t *readHeadStart=rowBuf->readPtr;

	kaelTui_getRowShapes( page, shapePtrList, currentRow);
   shapePtrCount = kaelTree_length(shapePtrList);

	KaelBook_shape *shapePtr = NULL;

	while( shapePtrIndex<=shapePtrCount && shapePtrCount!=0 ){

		//next shape
		if( shapePtr==NULL || (currentCol - shapePtr->pos[0]) >= shapePtr->size[0] ){ //shape is NULL OR currentCol exceeds shape 

			if( shapePtrIndex>=shapePtrCount ){ //if current==end
				break; 
			}
			
			//next shape pointer
			shapePtr = *(KaelBook_shape **)kaelTree_get(shapePtrList, shapePtrIndex);
			shapePtrIndex++;
			if(shapePtr==NULL){
				break;
			}

			//continue reading shape string at shape readhead
			rowBuf->readPtr = (uint8_t *)shapePtr->string + shapePtr->readHead;
			readHeadStart = (uint8_t *)rowBuf->readPtr;

			//jump to next shape
			if( currentCol < shapePtr->pos[0] ){
				uint16_t jumpCount = shapePtr->pos[0]-currentCol;
				kaelTui_printMarkerStyle(rowBuf, ansiReset);
				kaelTui_printeSpace(rowBuf, jumpCount);
				kaelTui_printMarkerStyle(rowBuf, shapePtr->lastStyle); 

				currentCol+=jumpCount;
			}

		}

		//if past last column
		if(currentCol>=page->vcols){
			break;
		}

		//Check if shapePtr has backlogged jumps
		if(shapePtr->jumpsRemaining!=0){
			kaelTui_printMarkerStyle(rowBuf, shapePtr->lastStyle);
			uint8_t shapeColsRemaining = shapePtr->size[0] - (currentCol - shapePtr->pos[0]);
			uint8_t jumpCount = kaelMath_min( shapePtr->jumpsRemaining, shapeColsRemaining ); //Can't exceed shape
			kaelTui_printeSpace(rowBuf, jumpCount);
			shapePtr->jumpsRemaining-=jumpCount;
			currentCol+=jumpCount;
			continue;
		}

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
				currentCol++;
				break;

			default: 
				//Regular characters (and unicode if it's ever supported)
				rowBuf->s[rowBuf->pos++] = rowBuf->readPtr[0];
				//non zero character increments
				currentCol++;
				kaelTui_printFullBuf(rowBuf,1); //Print if last char filled buffer
				rowBuf->readPtr++;
				break;
		}
	}

	//copy readhead progress to shape
	if(shapePtr!=NULL){
		uint16_t readCount = rowBuf->readPtr - readHeadStart;
		shapePtr->readHead+= readCount;
	}
			
	kaelTui_printMarkerStyle(rowBuf, ansiReset); 
	currentCol=0;

	//Check if enough space in rowBuf for '\n'
	kaelTui_printFullBuf(rowBuf,1);

	//Newline starts
	rowBuf->s[rowBuf->pos++] = '\n'; 
}



/**
	@brief print rectangle in viewport

	Private Use Area (PUA), 2-byte range: U+E000 to U+F8FF
	Linux ANSI color escape sequences are atrociously long, up to 8 bytes '\x1b'[1;37m
	Let us use unicode PUA BMP range U+E0_00 to U+F8_FF
	
	Check if the first byte falls in this range then it is a marker
	otherwise print unicode by the given length 'L', 
	number of leading 1s set the unicode length, [0b1L...0#] [0b10######] [0b10######] ...
	e.g. 0b11110000 has total 4 bytes, the trailing bits are part of the unicode
	
	Since 0xE0 to 0xF8 are 
*/	
void kaelTui_printPage( const KaelBook_page *page, KaelTui_RowBuffer *rowBuf ){
	if(kaelTree_empty(&page->shape) || page->vcols==0 || page->vrows==0){
		return;
	}

   //List of shape pointers that will be printed on current row
	KaelTree shapePtrList;
	kaelTree_alloc(&shapePtrList, sizeof(KaelBook_shape*));

	uint16_t currentCol=0;

	//print page row by row by searching overlapping shapes on each row
	for(uint16_t currentRow=0; currentRow<page->vrows; currentRow++){
		kaelTui_printRow( page, rowBuf, &shapePtrList, currentCol, currentRow );
	}
	//Print remaining buffer
	kaelTui_printRowBuf(rowBuf);

	//page ends
	kaelTree_free(&shapePtrList);
	fflush(stdout);
}

