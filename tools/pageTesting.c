



#include "kaelygon/treeMem/tree.h"
#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"
#include <x86intrin.h> 

#include <stdio.h>





typedef enum {
	/*Style*/
	RESET 		= 0, 
	BOLD 			= 1,  
	UNDERLINE 	= 4, 
	BLINK 		= 5, 
	REVERSE 		= 7, 
	HIDDEN 		= 2, //actual value is 8

	/*Foreground MOD_COL to get ansi code*/
	BLACK 		= 0,
	RED 			= 1, 
	GREEN 		= 2, 
	YELLOW 		= 3, 
	BLUE 			= 4, 
	MAGENTA 		= 5, 
	CYAN 			= 6, 
	WHITE 		= 7, 
} AnsiCode_enum;

typedef enum{
	FG_LO, 
	FG_HI, 
	BG_HI, 
	BG_LO, 
}AnsiMod_enum;

uint8_t AnsiMod[4] = {
	30, //FG_LO
	40, //FG_HI
	70, //BG_HI
	90, //BG_LO
};

/*

1 byte ANSI coding
mod | color | style
##  |  111  | 111

first 2 bits are modifier (mod)
00 = FG LO color
01 = FG HI color
00 = BG LO color
01 = BG HI color

0b (00 101 100) = underlined magenta
*/

typedef union {
	struct {
		uint8_t mod    : 2;
		uint8_t color  : 3;
		uint8_t style  : 3;
	};
	uint8_t code;
} AnsiStyle;

typedef struct{
	uint16_t pos[2];
	uint8_t size[2];

	char *text; //Null terminated string
}PageShape;
/*
	-The shapes are sorted by position and ideally there's no overlap.
	-Row buffer loop takes the first shape position as the start and copies its row to buffer. 
	-If buffer is full, it's printed out which after reading continues till last column of the page or char is NULL. 
	-Each shape read head is advanced by how much was written to the buffer. 
	-If there's no more shapes before last column, a newline is printed and reading starts from 0th shape but this time the read head is shifted and row buffer will print out on the next line.
	While looping, shapes which are fully printed out will be skipped since their read head position char is NULL. Read head could be just copy of char pointer which is incremented
	Once all shapes are printed, read heads are zeroed and a new frame begins

	-The row buffer loop needs some method to detect when to print next AnsiStyle and when to jump white spaces by advancing the read head and by much. 
*/
/* 
	Ansi escape sequences take up to 10 bytes but here they are stored in 1 byte, AnsiStyle.
	In string they take 3 bytes since they are detected by '\esc' followed by marker type and finally information related to the marker
	It's very much akin to ansi escape sequence, but compacted

	MARKER_STYLE and MARKER_JUMP are some otherwise unused ascii characters or escape sequences.
	Having for example \esc[NULL], \esc[MARKER_STYLE] would make the switch statement true or false case
	
	Example what one shape.text contains
	(char *)shape->text = {\esc,[MARKER_STYLE],[AnsiStyle_byte],H,e,l,l,o,\esc,[MARKER_JUMP],[Jump_byte],W,o,r,l,d,!,\esc,[NULL]};

	Pseudo code
	char *readHeadList[shapeCount]; //List of each shape read progress. Text wraps after new line
	uint16_t rowBufPos = 0;

	//reset each shape readHead
	i in shapeList.len()
		readHeadList[i] = (char *)shape[i].text

	//iterate 2D cols -> rows
	for( i=0; i < col; col++ ){ 
		for( j=0; j < row; row++ ){

			//Iterate each shape by index and read each shape till shape last column
			for( readHead in readHeadList ){

				if( rowBufPos >= rowBufPos.size ){ //print if buffer is full
					print rowBuffer;
					rowBufPos = 0;
				}

				while( 1 ){

					while( *readHead != '\esc' ){ Keep writing till escape sequence
						rowBuffer[rowBufPos] = *readHead; // Store byte to buffer
						
					}

					switch (*readHead) { 
						case MARKER_JUMP:
							readHead++; //skip marker
							rowBufPos+= *readHead; // Skip by count stored in Jump_byte
							break;
						case MARKER_STYLE:
							readHead++;
							insertAnsiStyle(*readHead); // Insert color escape sequence
							break;
						case NULL:
							goto nextShape; // break while loop
							break;
					}

					*readHead++; //advance byte

				}
				nextShape:

			}
			print('\n')
		}
	}

	New frame starts

*/

//These could be stored somewhere
void generateShapes(KaelTree *shapeList, uint16_t shapeCount){
	const uint8_t useResize = 0; //0= push, 1= resize
	const uint16_t maxSize = 256; //0= push, 1= resize

	kaelTree_alloc(shapeList, sizeof(PageShape));
	if(useResize){
		kaelTree_resize(shapeList, shapeCount);
	}

	uint16_t randNum = 0;
	for(uint16_t i=0; i<shapeCount; i++){
		PageShape tmpShape;
		randNum = kaelRand_lcg(randNum);
		tmpShape.pos[0]=(randNum      ) % maxSize;
		tmpShape.pos[1]=(randNum>>8   ) % maxSize;

		randNum = kaelRand_lcg(randNum);
		tmpShape.size[0]=(randNum    ) % (maxSize - tmpShape.pos[0]);
		tmpShape.size[1]=(randNum>>8 ) % (maxSize - tmpShape.pos[1]);

		if(useResize){
			kaelTree_set(shapeList, i, &tmpShape);
		}else{
			void *ptr = kaelTree_push(shapeList, &tmpShape);
			if(NULL_CHECK(ptr)){break;}
		}
	}
}


int main(){




	static char escSeq[10];

	AnsiStyle code = { .mod = FG_LO, .color = MAGENTA, .style = BOLD };
	
	// Special case for RESET, if style is reset we return the reset code
	if (code.style == RESET) {
		memcpy(escSeq,"\x1b[0m",5);
	}else{
		snprintf(escSeq, 10, "\x1b[%d;%dm", code.style, code.color + AnsiMod[code.mod]);
	}
	
	printf("%sHello world%s\n", escSeq, "\x1b[0m");

	return 0;


	//uint16_t viewPortPos[2]={0};
	//uint16_t viewPortSize[2]={0};
	
	uint16_t shapeCount = 64;

	KaelTree shapeList; 

	generateShapes(&shapeList, shapeCount);

	//uint8_t rowBuf[256];

	kaelTree_free(&shapeList);
	
}