/**
 * @file krleTGA.c
 * 
 * @brief header, Conversion tools to convert between TGA and KRLE (Kael Run Length Encoding)
 * 
 * Optimized for printing decoding bytes into ANSI escape codes rather than file size
 * Limited to 64kib-1 file size
 */

#include "krle/krleTGA.h"


//------ File I/O ------

/**
 * @brief TGA to krle encode function
 */
void krle_TGAToKRLE(const char *TGAFile, const char *KRLEFile, uint8_t stretchFactor, uint8_t sampleType){
	if(NULL_CHECK(TGAFile) || NULL_CHECK(KRLEFile)){
		return;
	}
	
	stretchFactor = stretchFactor==0 ? 1 : stretchFactor;
	uint8_t *TGAPixels = NULL;

	Krle_TGAHeader TGAHeader = krle_readTGAFile(TGAFile, &TGAPixels);
	uint32_t pixelsTotal = TGAHeader.width * TGAHeader.height;

	if(pixelsTotal==0){
		return;
	}

	//Convert orchis palette to LAB
	Krle_LAB orchisPaletteLAB[KRLE_PALETTE_SIZE];
	krle_paletteRGBToLAB(krle_orchisPalette, orchisPaletteLAB, KRLE_PALETTE_SIZE);
	

	//Convert TGA to KRLE string
	KaelTree krleTree = {0};
	kaelTree_alloc(&krleTree,sizeof(uint8_t));
	krle_pixelsToKRLE(&krleTree, orchisPaletteLAB, TGAPixels, TGAHeader.width, TGAHeader.height, stretchFactor, sampleType);
	free(TGAPixels);

	uint16_t squashedHeight = (TGAHeader.height+(stretchFactor-1))/stretchFactor; //ceil
	Krle_header KRLEHeader = krle_createKRLEHeader( TGAHeader.width, squashedHeight, kaelTree_length(&krleTree), stretchFactor);

	const uint8_t *KRLEString = kaelTree_get(&krleTree, 0);
	krle_writeKRLEFile(KRLEString, KRLEHeader, KRLEFile);
	kaelTree_free(&krleTree);


	//Info printing
	uint16_t compressedSize = KRLEHeader.length;
	uint32_t stretchedPixels = TGAHeader.width*TGAHeader.height/stretchFactor;
	float bytesPerPixel =  8.0*(float)compressedSize/(stretchedPixels);
	float pixelsPerByte = (float)stretchedPixels/compressedSize;
	printf("%u pixels converted to %u bytes\n", stretchedPixels, compressedSize);
	printf("%.2f pixels per byte\n", pixelsPerByte);
	printf("%.2f bits per pixel\n",bytesPerPixel);
}


/**
 * @brief KRLE to TGA conversion function
 */
void krle_KRLEToTGA(const char *KRLEFile, const char *TGAFile){
	if(NULL_CHECK(TGAFile) || NULL_CHECK(KRLEFile)){
		return;
	}

	uint8_t *KRLEString = NULL;
	Krle_header KRLEHeader = krle_readKRLEFile(KRLEFile, &KRLEString);
	
	uint8_t *TGAPixels = NULL;

	krle_KRLEToPixels(KRLEString, &TGAPixels, KRLEHeader);
	Krle_TGAHeader TGAHeader = krle_createTGAHeader(KRLEHeader.width, KRLEHeader.height*KRLEHeader.ratio);
	krle_writeTGAFile(TGAFile, TGAHeader, TGAPixels);

	free(TGAPixels);
	free(KRLEString);
}


//--- TGA ---

/**
 * @brief Read and copy TGA file BGRA32 pixels to a new allocation
 * 
 * @note TGAPixels must be freed after use
 */
Krle_TGAHeader krle_readTGAFile(const char *filePath, uint8_t **TGAPixels){
	FILE *file = fopen(filePath, "rb");
	if(!file) {
		printf("Failed to open %s",filePath);
		return (Krle_TGAHeader){0};
	}
	
	Krle_TGAHeader header;
	fread(&header, sizeof(Krle_TGAHeader), 1, file);
	
	if(header.bitsPerPixel != 32 || header.dataTypeCode != 2){
		fprintf(stderr, "Unsupported TGA format. Only 32-bit uncompressed supported.\n");
		fclose(file);
		return (Krle_TGAHeader){0};
	}
	
	*TGAPixels = calloc(4 * header.width * header.height, sizeof(uint8_t)); // 4 bytes per pixel BGRA
	if(NULL_CHECK(*TGAPixels)){
		return (Krle_TGAHeader){0};
	}
 	
	fread(*TGAPixels, 4, header.width * header.height, file);
	fclose(file);

	return header;
}

/**
 * @brief Write BGRA32 pixels to file
 */
void krle_writeTGAFile(const char* fileName, Krle_TGAHeader TGAHeader, uint8_t *TGAPixels){
	FILE *outFile = fopen(fileName, "wb");
	if(!outFile){
		printf("Failed to open %s\n", fileName);
		return;
	}
	fwrite(&TGAHeader, sizeof(Krle_TGAHeader), 1, outFile);
	fwrite(TGAPixels, 4*(TGAHeader.width * TGAHeader.height)*sizeof(uint8_t), 1, outFile); //4 bytes per pixel
	fclose(outFile);
}

/**
 * @brief Create default TGA BGRA32 header template
 */
Krle_TGAHeader krle_createTGAHeader(uint16_t width, uint16_t height){
	Krle_TGAHeader TGAHeader = {0};
	TGAHeader.dataTypeCode = 2;
	TGAHeader.height = height;
	TGAHeader.width = width;
	TGAHeader.bitsPerPixel = 32; 
	TGAHeader.imageDescriptor = 40;

	return TGAHeader;
}


//--- KRLE ---

/**
 * @brief Read and copy KRLE file into string
 * 
 * @note KRLEString must freed after use
 */
Krle_header krle_readKRLEFile(const char *filePath, uint8_t **KRLEString){
	FILE *file = fopen(filePath, "rb");
	if(!file) {
		printf("Failed to open %s",filePath);
		return (Krle_header){0};
	}
	
	Krle_header header;
	fread(&header, sizeof(Krle_header), 1, file);
	
	if(header.length == 0){
		fprintf(stderr, "Invalid byte string length\n");
		fclose(file);
		return (Krle_header){0};
	}
	
	*KRLEString = calloc(header.length*sizeof(uint8_t), 1);
	if(NULL_CHECK(*KRLEString)){
		return (Krle_header){0};
	}
 	
	fread(*KRLEString, sizeof(uint8_t), header.length, file);
	fclose(file);

	return header;
}

/**
 * @brief Write krle string to file
 */
void krle_writeKRLEFile(const uint8_t *KRLEString, Krle_header KRLEHeader, const char* fileName){

	FILE *outFile = fopen(fileName, "wb");
	if(!outFile){
		printf("Failed to open %s\n", fileName);
		return;
	}
	fwrite(&KRLEHeader, sizeof(Krle_header), 1, outFile);
	fwrite(KRLEString, KRLEHeader.length*sizeof(uint8_t), 1, outFile); //4 bytes per pixel
	fclose(outFile);
}

/**
 * @brief Create default krle header template
 */
Krle_header krle_createKRLEHeader(uint16_t width, uint16_t height, uint32_t length, uint8_t ratio){
	Krle_header KRLEHeader 	= {0};
	KRLEHeader.width			= width;
	KRLEHeader.height			= height;
	for(uint32_t i=0; i<sizeof(krle_defaultColorTable);i++){
		KRLEHeader.palette[i]	= krle_defaultColorTable[i];
		
	}
	for(uint32_t i=0; i<sizeof(krle_defaultAttributeTable);i++){
		KRLEHeader.attribute[i]	= krle_defaultAttributeTable[i];
	}
	KRLEHeader.ratio			= ratio==0 ? 2 : ratio;
	KRLEHeader.length			= length==0 ? width*height : length;
	KRLEHeader.flags.byte	= KRLE_PIXEL_MODE_FLAG;

	return KRLEHeader;
}






//------ Convert 32bit BGRA pixels to KRLE string ------

/**
 * @brief Append jump run to krle string
 */
void krle_packJumpRun(KaelTree *krleTree, uint32_t *jumpLength, uint32_t maxJump){
	#if KRLE_EXTRA_DEBUGGING==1
		printf("Jump runs ");
	#endif
	while(*jumpLength){
		uint8_t runLength = kaelMath_min(*jumpLength, maxJump);
		kaelTree_push(krleTree,&(uint8_t){KRLE_PIXEL_JUMP});
		kaelTree_push(krleTree,&(uint8_t){runLength});
		
		#if KRLE_EXTRA_DEBUGGING==1
			printf("%d ",runLength);
		#endif
		*jumpLength -= runLength;
	}
	
	#if KRLE_EXTRA_DEBUGGING==1
		printf("\n");
	#endif
}

/**
 * @brief Append pixel run to krle string
 */
void krle_packPixelRun(KaelTree *krleTree, uint8_t paletteIndex, uint32_t *pixelLength, uint32_t maxPixelLength){
	//Chain of same pixels ended
	#if KRLE_EXTRA_DEBUGGING==1
		printf("palette %u runs ", paletteIndex);
	#endif

	while(*pixelLength){
		uint8_t runLength = kaelMath_min(*pixelLength, maxPixelLength);
		uint8_t pixelByte = kaelMath_u8pack(paletteIndex, runLength);
		kaelTree_push(krleTree,&(uint8_t){pixelByte});

		#if KRLE_EXTRA_DEBUGGING==1
			printf("%d ",runLength);
		#endif
		*pixelLength -= runLength;
	}
	
	#if KRLE_EXTRA_DEBUGGING==1
		printf("\n");
	#endif
}

/**
 * @brief Unpack pixel run
 */
void krle_unpackPixelRun(uint8_t byte, uint8_t *paletteIndex, uint8_t *length ){
	kaelMath_u8unpack(byte, paletteIndex, length); //High nibble is read first
}



//------ RLE Detection Incrementor ------
typedef struct {
	uint32_t i, x, y;
	const uint32_t addx, addy;
	const uint32_t height, width;
	uint8_t isWithinRange;
}Krle_iterator;

/**
 * @brief Is current row in canvas && fail safe
 */
uint8_t krle_isWithinRange(const Krle_iterator *it){
	return it->isWithinRange;
}

/**
 * @brief Increment flat array as 2D plane. Return state before addend
 */
uint8_t krle_increment(Krle_iterator *it){
	if(NULL_CHECK(it) || !it->isWithinRange){
		return 0;
	}
	it->x += it->addx;
	if(it->x >= it->width){
		it->y += it->addy;
		it->x  = 0;
	}
	it->i = it->y * it->width + it->x;
	it->isWithinRange = (it->y < it->height)/* && (it->i < it->width * it->height)*/;
	return it->isWithinRange;
}

/**
 * @brief encode TGA (BGRA32) pixels into krle string stored as KaelTree
 * 
 */
void krle_pixelsToKRLE(KaelTree *krleTree, const Krle_LAB *labPalette, const uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint8_t stretchFactor, uint8_t sampleType){
	if(NULL_CHECK(krleTree) || NULL_CHECK(labPalette) || NULL_CHECK(TGAPixels) ){
		return;
	}
	stretchFactor = stretchFactor==0 ? 1 : stretchFactor;

	//Raw 5 bit colors per pixel (4bit colors 1bit alpha), 1.6 pixels in byte
	//Converted fromat fits 3-2 pixels into one byte. Worst case scenario ~8 bits in one byte  
	kaelTree_setWidth(krleTree,sizeof(uint8_t));
	kaelTree_reserve(krleTree, TGAWidth * TGAHeight / (2*stretchFactor));

	const uint32_t maxJump=UINT8_MAX;
	const uint32_t maxPixelLength=15;
	const uint8_t alphaClip=127; //pixel is discarded at this alpha value or lower

	uint32_t pixelCount = 0; //for debugging
	uint32_t pixelLength=0; //How long pixel run
	uint8_t prevPixel = 0xFF;
	uint32_t jumpLength=0;

	Krle_iterator px = (Krle_iterator){
		.i = 0, .x = 0, .y = 0,
		.addx = 1, .addy = stretchFactor,
		.width = TGAWidth, .height = TGAHeight,
		.isWithinRange = 1,
	};

	//If first pixel is transparent, it's a jump run
	uint8_t isJumpRun = TGAPixels[0*4+3]<=alphaClip; //1==Jump Run, 0=Pixel Run

	/**
	 * Detect whether pixel is transparent or opaque. 
	 * When run ends, switch to other sub-loop without incrementing as both loops need to read the same pixel that broke a run
	 * stretchFactor=2 means that every odd row is skipped to "squish" the pixels as terminal characters are roughly 2 tall 1 wide
	 * 
	 * T=transparent, O=opaque
	 * Jump loop
	 * Read and increment i [T, i++, T, i++, T, i++, O, break] 
	 * (Switch to run loop) 
	 * Read and increment i [O, i++, O, i++, O, i++, T, break] 
	 * (Switch to jump loop)
	 */
	while(krle_isWithinRange(&px)){
		uint8_t alpha = TGAPixels[px.i*4+3];
		uint8_t isTransparent = alpha<=alphaClip;

		if(isJumpRun){
			//Jump runs sub-loop
			if(isTransparent){
				//Transparent
				jumpLength++;
			}else
			if(jumpLength){
				//Opaque; Jump run ends
				pixelCount+=jumpLength;
				krle_packJumpRun(krleTree, &jumpLength, maxJump);

				//break without increment
				isJumpRun=0;
				continue;
			}
		
		}else{
			//Pixel runs sub-loop
			uint8_t thisPixel = 0xFF;

			if(!isTransparent){
				//Opaque
				//Sample pixel
				Krle_LAB LABTriple = KRLE_MAGENTA_LAB;

				if(sampleType==KRLE_LAB_AVG && stretchFactor > 1){
					LABTriple = krle_LABAvgRow(TGAPixels, TGAWidth, TGAHeight, px.i, stretchFactor);
				}else
				if(sampleType==KRLE_BILINEAR && stretchFactor > 1){
					LABTriple = krle_bilinearRow(TGAPixels, TGAWidth, TGAHeight, px.i, stretchFactor);
				}else{
					//Default 0, KRLE_NEAREST_NEIGHBOR
					LABTriple = krle_nearestRow(TGAPixels, TGAWidth, TGAHeight, px.i, stretchFactor);
				}

				thisPixel = krle_palettizeLAB(labPalette, LABTriple, KRLE_PALETTE_SIZE);
				#if KRLE_EXTRA_DEBUGGING==1
					krle_debugColorDistance(LABTriple, krle_orchisPalette, thisPixel);
				#endif
			}

			if((thisPixel!=prevPixel && pixelLength) || isTransparent){
				//run longer than 1 ended OR next pixel is transparent
				pixelCount+=pixelLength;
				krle_packPixelRun(krleTree, prevPixel, &pixelLength, maxPixelLength);
			}
			
			if(isTransparent){
				//break without increment
				isJumpRun=1;
				continue;
			}

			pixelLength++; //Add thisPixel to next run
			prevPixel = thisPixel;
		}

		krle_increment(&px);
	}

	//clear remaining pixels. Trailing jumps are ignored
	if(prevPixel!=0xFF && pixelLength){
		pixelCount+=pixelLength;
		krle_packPixelRun(krleTree, prevPixel, &pixelLength, maxPixelLength);
	}
	
	//May differ from actual size since trailing jumps are ignored
	printf("Wrote %d pixels into KRLE string\n",pixelCount);

	//Null terminate
	uint8_t *newElem = kaelTree_push(krleTree,&(uint8_t){'\0'});
	if(NULL_CHECK(newElem)){
		//push failed due to being full
		uint16_t lastIndex = kaelTree_length(krleTree)-1;
		kaelTree_set(krleTree, lastIndex, &(uint8_t){'\0'} );
	}
}






//------ KRLE to 32bit BGRA pixels ------

/**
 * @brief Copy colors and repeat each row header.ratio times
 * 
 * For example, if header.ratio==2, even lines are repeated twice effectively stretching the image height by 2x
 */
uint8_t krle_applyPixelRatio(uint8_t *TGAPixels, Krle_header header, uint32_t *px, uint8_t R, uint8_t G, uint8_t B, uint8_t A){
	for(uint16_t j=0; j<header.ratio; j++){
		if(*px > header.width * header.height * header.ratio){
			return KRLE_ERR_PIXEL_OVERFLOW;
		}
		//Repeat pixels on next rows
		uint32_t yOffset = j*header.width; 
		TGAPixels[(*px+yOffset)*4+2] = R;
		TGAPixels[(*px+yOffset)*4+1] = G;
		TGAPixels[(*px+yOffset)*4+0] = B;
		TGAPixels[(*px+yOffset)*4+3] = A;
	}

	*px+=1;
	if(*px % header.width==0){
		//Whenever last column is reached, we skip the repeated rows
		*px += (header.ratio-1) * header.width;
	}
	return KRLE_SUCCESS;
}

/**
 * @brief Convert KRLE Jump to pixels
 */
uint8_t krle_jumpToPixels(uint8_t *TGAPixels, Krle_header header, uint8_t jumpLength, uint32_t *px){
	if(jumpLength==0){
		printf("Pixel %u Invalid Jump Length\n", *px);
		return KRLE_ERR_ZERO_JUMP;
	}
	for(uint16_t i=0; i<jumpLength; i++){
		//Transparent white
		uint8_t code = krle_applyPixelRatio(TGAPixels, header, px, 255, 255, 255, 0);
		if(code!=KRLE_SUCCESS){
			return code;
		}
	}
	return KRLE_SUCCESS;
}


/**
 * @brief Convert KRLE run to pixels
 */
uint8_t krle_runToPixels(uint8_t *TGAPixels, Krle_header header, uint8_t byte, uint32_t *px){
	uint8_t paletteIndex;
	uint8_t length;
	
	krle_unpackPixelRun(byte, &paletteIndex, &length);
	if(length==0){
		printf("Pixel %u Invalid Run Length\n", *px);
		return KRLE_ERR_ZERO_JUMP;
	}
	for(uint16_t i=0; i<length; i++){
		uint8_t code = krle_applyPixelRatio(
			TGAPixels, header, px, 
			krle_orchisPalette[paletteIndex].r,
			krle_orchisPalette[paletteIndex].g,
			krle_orchisPalette[paletteIndex].b,
			255
		);
		if(code!=KRLE_SUCCESS){
			return code;
		}
	}
	return KRLE_SUCCESS;
}



/**
 * @brief Convert KRLE null terminated byte string into 32-bit BGRA pixels
 * 
 * @note TGAPixels must have allocation of 4*width*height bytes, OR left NULL to create a new allocation of that size
 */
uint32_t krle_KRLEToPixels(const uint8_t *KRLEString, uint8_t **TGAPixels, const Krle_header header){
	if(NULL_CHECK(KRLEString)){
		printf("NULL input in krle_KRLEToPixels()\n");
		return 0;
	}

	uint32_t totalPixels = header.width * header.height * header.ratio;
	uint32_t px=0; //pixel index

	if( NULL_CHECK(*TGAPixels)){
		*TGAPixels = calloc( 4*totalPixels, sizeof(uint8_t)); //32bits per pixel stretched by header.ratio
		if(NULL_CHECK(*TGAPixels)){
			printf("TGAPixels failed to alloc\n");
			return 0;
		}
	}


	const uint8_t *readHead = KRLEString;

	uint8_t code=KRLE_SUCCESS;

	while( px<totalPixels && readHead[0]!=0){
		switch(readHead[0]){
			case KRLE_PIXEL_JUMP:
				readHead++; //Skip marker
				code = krle_jumpToPixels(*TGAPixels, header, readHead[0], &px);
				break;

			default:
				code = krle_runToPixels(*TGAPixels, header, readHead[0], &px);
				break;
		}
		
		if(code!=KRLE_SUCCESS){
			//Illegal things by KRLE standards
			if(code==KRLE_ERR_ZERO_JUMP){
				printf("Zero jump is not allowed");
			}else
			if(code==KRLE_ERR_PIXEL_OVERFLOW){
				printf("Read too many pixels from KRLE string\n");
			}
			break;
		}
		readHead++;
	}


	printf("Read %u pixels from KRLE string\n",px);
	return px;
}



