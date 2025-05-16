/*

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

*/