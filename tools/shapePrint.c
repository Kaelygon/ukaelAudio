
#include "kaelygon/treeMem/tree.h"
#include <x86intrin.h> 

#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

#include <stdio.h>

/**
 * @brief fancy string print buffer
 */
typedef struct{
	uint8_t* s; //buffer address 
	uint16_t pos; //Write position of .s
	uint16_t size; //buffer .s size
	const uint8_t* read; //string char index being read
}KaelTui_RowBuffer;


typedef enum {
	/*Style*/
	ansiReset 		= 0xFF, // we can't have null data
	ansiBold 		= 1,  
	ansiUnderline 	= 4, 
	ansiBlink 		= 5, 
	ansiReverse 	= 7, 
	//ansiHidden	= 8, overflow. Use ansiWhite space or jump instead

	/*Foreground MOD_COL to get ansi code*/
	ansiBlack 	= 0,
	ansiRed 		= 1, 
	ansiGreen 	= 2, 
	ansiYellow 	= 3, 
	ansiBlue 	= 4, 
	ansiMagenta = 5, 
	ansiCyan 	= 6, 
	ansiWhite 	= 7, 

	ansiLength 	= 7, //Maximum bytes decoded ansi esc seq can take
} KaelTui_ansiColor;

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

//indices of the table AnsiModValue[]
typedef enum{
	ansiFGLow, 
	ansiFGHigh, 
	ansiBGLow, 
	ansiBGHigh, 
}KaelTui_ansiMod;

/**
 * @brief Ansi color code escape sequence encoded in a single byte
 */
typedef union {
	struct {
		uint8_t mod    : 2;
		uint8_t color  : 3;
		uint8_t style  : 3;
	};
	uint8_t byte;
} KaelTui_ansiCode;

/**
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
 * 
 * Formatting in string
 * | [marker]		| [data byte]     |
 * | -----------  | --------------- |
 * | markerJump	| [jump length]   |
 * | markerStyle	| [ansiCode.byte] |
 * 
 */
typedef enum{
	markerJump = 0b11100000,
	markerStyle = 0b11100001,
}KaelTui_Marker;



//------ Ansi escape sequence ------


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


void kaelTui_printRowBuf(KaelTui_RowBuffer *rowBuf){
	KAEL_ASSERT(rowBuf!=NULL);
	//Print only up to overwritten part
	fwrite((char *)rowBuf->s, sizeof(char), rowBuf->pos, stdout); 
	rowBuf->pos=0;
	return;
}

/**
 * @brief Print white spaces
*/
void kaelTui_fillJump(KaelTui_RowBuffer *rowBuf, uint16_t jumpCount){
	KAEL_ASSERT(rowBuf!=NULL);
	KAEL_ASSERT(rowBuf!=0);
	if(jumpCount==0){
		return;  //NULL data
	};

	for(uint16_t i=0; i<jumpCount; i++){ //jump
		if(rowBuf->pos >= rowBuf->size){
			kaelTui_printRowBuf(rowBuf);
		}
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
	rowBuf->pos+=kaelTui_encodeAnsiEsc(rowBuf->s, rawByte, rowBuf->pos);
	return;
}


typedef struct{
	uint16_t pos[2]; //col by row
	uint16_t size[2];
	uint8_t *string;
	uint16_t readHead;
}KaelBook_shape;

typedef struct{
	KaelTree shape;
	uint16_t vcols; //virtual columns
	uint16_t vrows; //virtual rows
}KaelBook_page;



//fetch page shapes on row
uint16_t kaelTui_getRowShapes( const KaelBook_page *page, KaelTree *shapeList, uint16_t row ){
	uint16_t shapeCount=0;
	KaelBook_shape *shape;

	for(uint16_t i=0; i < kaelTree_length(&page->shape); i++){ 
		shape = kaelTree_get(&page->shape, i);

		uint16_t shapeStartRow = shape->pos[1];
		uint16_t shapeEndRow = shape->pos[1] + shape->size[1];
		uint8_t condition = row >= shapeStartRow && row < shapeEndRow; //Copy pointer to pointer

		if( condition ){ //If shape overlaps with row
			void* isValid = kaelTree_push(shapeList, &i); 
			if(isValid){
				shapeCount++;
			}
		}

	}
	return shapeCount;
}

void kaelTui_printRow( const KaelBook_page *page, KaelTui_RowBuffer *rowBuf, KaelTree *shapeIndexList, uint8_t * readHeadStart, uint16_t currentCol, uint16_t currentRow ){
	//Reset index for each row
	kaelTree_resize(shapeIndexList, 0);

	uint16_t listIndex=0;
	uint16_t indexCount=0;

	indexCount = kaelTui_getRowShapes( page, shapeIndexList, currentRow);

	uint16_t shapeColPos=0;
	KaelBook_shape *shape = NULL;

	while( listIndex<=indexCount && indexCount!=0 ){

		//next shape
		if( shape==NULL || shapeColPos >= shape->size[0] ){ 

			if( listIndex>=indexCount){
				break; 
			}
			shapeColPos=0;
			
			//copy readhead progress to shape
			if(shape!=NULL){
				shape->readHead+= rowBuf->read - readHeadStart;
			}
			
			//next shape
			uint16_t shapeIndex = *(uint16_t *)kaelTree_get(shapeIndexList, listIndex);
			shape = (KaelBook_shape *)kaelTree_get(&page->shape, shapeIndex);
			listIndex++;
			if(shape==NULL){
				break;
			}

			//continue reading shape string at shape readhead
			rowBuf->read = (uint8_t *)shape->string + shape->readHead;
			readHeadStart = (uint8_t *)rowBuf->read;

			//jump to next shape
			if( currentCol < shape->pos[0] ){
				uint16_t jumpCount = shape->pos[0]-currentCol;
				kaelTui_printMarkerStyle(rowBuf, ansiReset);
				kaelTui_fillJump(rowBuf, jumpCount);

				currentCol+=jumpCount;
			}

		}

		//if past last column
		if(currentCol>=page->vcols){
			break;
		}

		//Parse character
		switch( (uint8_t)rowBuf->read[0] ){
			case markerJump:
				//Print number of spaces
				rowBuf->read++; //skip the marker
				uint8_t jumpCount = kaelMath_min( rowBuf->read[0], page->vcols - currentCol );
				kaelTui_fillJump(rowBuf, jumpCount);
				currentCol+=jumpCount;
				break;

			case markerStyle:
				//Ansi style and color encoding
				rowBuf->read++;
				kaelTui_printMarkerStyle(rowBuf, rowBuf->read[0]);
				break;

			case 0: 
				//NULL, invoke nextshape
				shape=NULL;
				break;

			default: 
				//Regular characters (and unicode if it's ever supported)
				rowBuf->s[rowBuf->pos++] = rowBuf->read[0];
				//non zero character increments
				currentCol++;
				shapeColPos++;
				break;
		}
		rowBuf->read++;

		//reserve ansiEscSeq + ansiReset + '\n' 
		if( rowBuf->pos+2*ansiLength >= rowBuf->size ){
			kaelTui_printRowBuf(rowBuf);
		}

	}

	//copy readhead progress to shape
	if(shape!=NULL){
		shape->readHead+= rowBuf->read - readHeadStart;
	}
			
	kaelTui_printMarkerStyle(rowBuf, ansiReset);
	currentCol=0;

	//Newline starts
	rowBuf->s[rowBuf->pos++] = '\n';
	kaelTui_printRowBuf(rowBuf);
}


/**
	@brief print rectangle in viewport

	Private Use Area (PUA), 2-byte range: U+E000 to U+F8FF
	Linux ANSI color escape sequences are atrociously long, up to 8 bytes '\x1b'[1;37m
	Let us use unicode PUA BMP range U+E0_00 to U+F8_FF
	
	Check if the first byte falls in this range it is a marker
	otherwise print unicode by the given length 'L', 
	number of leading 1s set the unicode length, [0b1L...0#] [0b10######] [0b10######] ...
	e.g. 0b11110000 has total 4 bytes, the trailing bits are part of the unicode
	
	Since 0xE0 to 0xF8 are 
*/	
void kaelTui_printPage( const KaelBook_page *page, KaelTui_RowBuffer *rowBuf ){
	if(kaelTree_empty(&page->shape) || page->vcols==0 || page->vrows==0){
		return;
	}

	KaelTree shapeIndexList;
	kaelTree_alloc(&shapeIndexList, sizeof(uint16_t));

	uint16_t currentCol=0;
	uint8_t *readHeadStart=0x0;

	//print page row by row by searching overlapping shapes on each row
	for(uint16_t currentRow=0; currentRow<page->vrows; currentRow++){
		kaelTui_printRow( page, rowBuf, &shapeIndexList, readHeadStart, currentCol, currentRow );
	}

	//page ends
	kaelTree_free(&shapeIndexList);
	fflush(stdout);
}

KaelBook_shape unit_genRandomShape(uint8_t randNum[3], const uint16_t cols, const uint16_t rows){
	KaelBook_shape shape=(KaelBook_shape){0};


	kaelRand_lcg24(randNum);
	shape.pos[0]	= randNum[0]%cols;
	shape.pos[1]	= randNum[1]%rows;

	uint16_t maxCols = cols-shape.pos[0];
	uint16_t maxRows = rows-shape.pos[0];

	maxCols+= maxCols==0;
	maxRows+= maxRows==0;

	shape.size[0]	= randNum[2]%(maxCols)+1;
	kaelRand_lcg24(randNum);
	shape.size[1]	= randNum[0]%(maxRows)+1;

	uint16_t charCount = 3 * shape.size[0] * shape.size[1];

	//Allocate shape string
	uint8_t *tmpString = calloc(charCount+1, sizeof(uint8_t));
	if(tmpString==NULL){return (KaelBook_shape){0};}

	for(uint32_t i=0; i<charCount; i++){
		uint8_t symbol=0;
		uint8_t legalString=1;

		kaelRand_lcg24(randNum);
		symbol = randNum[2];
		symbol+= symbol==0;

		if(i%3==0){
			tmpString[i++]=markerStyle;
			if(i>=charCount){
				break;
			}
			tmpString[i]=symbol;
		}else{
			if(legalString){
				uint8_t type = (randNum[1]>>6)&0b11;
				switch(type){
					case 0:
						symbol = symbol*('9'-'0')/255 + '0';
						break;
					case 1:
						symbol = symbol*('Z'-'A')/255 + 'A';
						break;
					case 2:
						symbol = symbol*('z'-'a')/255 + 'a';
						break;
					case 3:
						symbol = ' ';
						break;
				}
			}
			tmpString[i] = symbol;
		}
	}
	if(charCount){
		tmpString[charCount]='\0';
	}
	shape.string = tmpString;

	//for(uint16_t i=0; i<strlen((char *)tmpString); i++){
	//	printf("%d ", tmpString[i]);
	//}

	return shape;
}




//------ Unit test ------

KaelBook_shape unit_genCheckboardShape(uint8_t randNum[3], const uint16_t posRow, const uint16_t posCol, const uint16_t sizeRow, const uint16_t sizeCol ){
	KaelBook_shape shape=(KaelBook_shape){0};

	kaelRand_lcg24(randNum);
	shape.pos[0]	= posRow;
	shape.pos[1]	= posCol;

	shape.size[0]	= sizeRow;
	shape.size[1]	= sizeCol;

	uint16_t symbols = 3 * shape.size[0] * shape.size[1];

	//Allocate shape string
	uint8_t *tmpString = calloc(symbols+1, sizeof(uint8_t));
	if(tmpString==NULL){return (KaelBook_shape){0};}

	//non-zero width character count
	uint16_t nzwChar = 0;
	
	uint8_t gridSize[2] = {4,2};
	for(uint16_t i=0; i<symbols; i++ ){
		uint16_t x = nzwChar%shape.size[0];
		uint16_t y = nzwChar/shape.size[0];
		uint8_t condition = (x/gridSize[0] + y/gridSize[1])%2;

		KaelTui_ansiCode stStyle = { .mod=AnsiModValue[ansiBGHigh], .color=ansiMagenta, .style=ansiBold };
		KaelTui_ansiCode ndStyle = { .mod=AnsiModValue[ansiBGHigh], .color=ansiWhite  , .style=ansiBold };

		if(i%3==0){
			tmpString[i++]=markerStyle;
			if(i>=symbols){
				break;
			}
			tmpString[i] = condition ? stStyle.byte : ndStyle.byte;
			
		}else{

			tmpString[i] = condition ? '#' : '@';
			nzwChar++;

			//printf("%c", tmpString[i]);
			//if(x==shape.size[0]-1){printf("\n");}
		}
	}
	//printf("\n\n");
	tmpString[symbols]='\0';
	shape.string = tmpString;

	return shape;
}



void unit_kaelTuiPrintPage(uint8_t randNum[3]){

	KaelBook_page page;
	page.vcols = 128;
	page.vrows = 24;

	kaelTree_alloc(&page.shape, sizeof(KaelBook_shape));

	//Generate shapes
	KaelBook_shape tmpShape;
	
	tmpShape = unit_genCheckboardShape(randNum,0,0,16,8);
	kaelTree_push(&page.shape, &tmpShape);

	tmpShape = unit_genCheckboardShape(randNum,20,10,12,6);
	kaelTree_push(&page.shape, &tmpShape);

	for(uint8_t i=0; i<3; i++){
		tmpShape = unit_genRandomShape(randNum, page.vcols, page.vrows);
		kaelTree_push(&page.shape, &tmpShape);
	}

	KaelTui_RowBuffer rowBuf = {
		.s = (uint8_t[256]){0},
		.read = NULL,
		.pos = 0,
		.size = 256
	};

	uint64_t startTime = __rdtsc();
	kaelTui_printPage(&page, &rowBuf);
	uint64_t endTime = __rdtsc();

	while(!kaelTree_empty(&page.shape)){
		KaelBook_shape *tmpShape = kaelTree_back(&page.shape);
		free(tmpShape->string);
		kaelTree_pop(&page.shape);
	}
	kaelTree_free(&page.shape);

	uint8_t ansiLen = 8;
	uint8_t ansiResetColor[ansiLen];
	memset(ansiResetColor,0,ansiLen);
	kaelTui_encodeAnsiEsc(ansiResetColor, ansiReset, 0);
	printf("%s\ntime %lu\n", ansiResetColor, endTime-startTime);
	
}

int main() {

	uint8_t randNum[3]={31,82,84};
	unit_kaelTuiPrintPage(randNum);

	return 0;
}



