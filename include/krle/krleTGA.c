/**
 * @file krleTGA.c
 * 
 * @brief header, Conversion tools to convert between TGA and KRLE (Kael Run Length Encoding)
 * 
 * Optimized for printing decoding bytes into ANSI escape codes rather than file size
 */

#include "krle/krleTGA.h"


/**
 * @brief {R, G, B} Tilix Orchis ANSI colors
 */
uint8_t krle_orchisPalette[16][3] = {
	{  0,   0,   0}, //ansiBlack
	{204,   0,   0}, //ansiRed
	{ 77, 154,   5}, //ansiGreen
	{195, 160,   0}, //ansiYellow
	{ 52, 100, 163}, //ansiBlue
	{117,  79, 123}, //ansiMagenta
	{  5, 151, 154}, //ansiCyan
	{211, 214, 207}, //ansiWhite

	{ 84,  86,  82}, //ansiBrightBlack
	{239,  40,  40}, //ansiBrightRed
	{137, 226,  52}, //ansiBrightGreen
	{251, 232,  79}, //ansiBrightYellow
	{114, 158, 207}, //ansiBrightBlue
	{172, 126, 168}, //ansiBrightMagenta
	{ 52, 226, 226}, //ansiBrightCyan
	{237, 237, 235}, //ansiBrightWhite	
};

//------ File I/O ------

//--- TGA ---

/**
 * @brief Read and copy TGA file BGRA32 pixels to a new allocation
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
Krle_TGAHeader krle_createTGAHeader(uint16_t height, uint16_t width){
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




//------ LAB conversions ------

static float krle_pivotRGB(float n) {
	n /= 255.0f;
	return (n > 0.04045f) ? powf((n + 0.055f) / 1.055f, 2.4f) : (n / 12.92f);
}

static float krle_pivotXYZ(float n) {
	return (n > 0.008856f) ? powf(n, 1.0f/3.0f) : (7.787f * n) + (16.0f / 116.0f);
}

/**
 * @brief RGB to LAB color conversion
 */
Krle_LAB krle_rgbToLab(Krle_RGB rgbPalette) {
	// Linearize RGB
	float R = krle_pivotRGB((float)rgbPalette.r);
	float G = krle_pivotRGB((float)rgbPalette.g);
	float B = krle_pivotRGB((float)rgbPalette.b);

	// Convert to XYZ (D65)
	float X = R * 0.4124f + G * 0.3576f + B * 0.1805f;
	float Y = R * 0.2126f + G * 0.7152f + B * 0.0722f;
	float Z = R * 0.0193f + G * 0.1192f + B * 0.9505f;

	// Normalize by D65 reference white
	X /= 0.95047f;
	Y /= 1.00000f;
	Z /= 1.08883f;

	// Convert to LAB
	X = krle_pivotXYZ(X);
	Y = krle_pivotXYZ(Y);
	Z = krle_pivotXYZ(Z);

	Krle_LAB lab;
	lab.l = (116.0f * Y) - 16.0f;
	lab.a = 500.0f * (X - Y);
	lab.b = 200.0f * (Y - Z);
	return lab;
}

float krle_labDistance(Krle_LAB lab1, Krle_LAB lab2){
	float dl = lab1.l - lab2.l;
	float da = lab1.a - lab2.a;
	float db = lab1.b - lab2.b;
	return dl * dl + da * da + db * db;
}

float krle_rgbDistance(Krle_RGB rgb1, Krle_RGB rgb2){
	Krle_LAB lab1 = krle_rgbToLab(rgb1);
	Krle_LAB lab2 = krle_rgbToLab(rgb2);
	return krle_labDistance(lab1, lab2);
}





//------ Helpers and debug tools ------

/**
 * @brief Convert RGB24 palette to LAB
 */

void krle_paletteRGBToLAB(Krle_LAB *labPalette, uint8_t rgbPalette[16][3]){
	for (int i = 0; i < 16; i++){
		Krle_RGB rgbTriple = (Krle_RGB){
			rgbPalette[i][0],
			rgbPalette[i][1],
			rgbPalette[i][2]
		};
		labPalette[i] = krle_rgbToLab(rgbTriple);
	}
}

/**
 * @brief Find nearest color to RGB triple
 */
int krle_palettizeRGB(Krle_LAB *labPalette, Krle_RGB rgbColor) {
	Krle_LAB original = krle_rgbToLab(rgbColor);
	float minDist = INFINITY;
	int minIndex = 0;

	for (int i = 0; i < 16; i++) {
		 float dist = krle_labDistance(original, labPalette[i]);

		 if (dist < minDist) {
			  minDist = dist;
			  minIndex = i;
		 }
	}

	return minIndex;
}

/**
 * @brief Convert RGB triple to LAB and calculate distance with palette LAB triple by index
 */
void krle_debugColorDistance(Krle_RGB rgbTriple, uint8_t palette[16][3], uint8_t index){
	Krle_RGB newTriple = (Krle_RGB){
		palette[index][0],
		palette[index][1],
		palette[index][2]					
	};
	float colorDistance = krle_rgbDistance(newTriple, rgbTriple);
   printf("palette %u delta %.2f\n", index, colorDistance);
}

void unit_krle_printRGB24Pixels(uint8_t *TGAPixels, uint32_t pixelsTotal){
	for(uint32_t i=0; i<pixelsTotal; i++){
		uint8_t b = TGAPixels[i*4+0];
		uint8_t g = TGAPixels[i*4+1];
		uint8_t r = TGAPixels[i*4+2];
		printf("{%u, %u, %u},\n",r,g,b);
	}
	return;
}





//------ Convert 32bit BGRA pixels to KRLE string ------

/**
 * @brief Toggle between pixel modes
 */
void krle_switchPixelMode(KaelTree *krleFormat, uint8_t *currentMode, uint8_t newMode ){
	if(*currentMode==newMode){
		//No switching
		return;
	}
	switch(newMode){
		case KRLE_PIXEL_RUN:
			*currentMode=KRLE_PIXEL_RUN;
			kaelTree_push(krleFormat,&(uint8_t){KRLE_PIXEL_RUN});
			break;
		case KRLE_PIXEL_PAIR:
			*currentMode=KRLE_PIXEL_PAIR;
			kaelTree_push(krleFormat,&(uint8_t){KRLE_PIXEL_PAIR});
			break;
	}
}

/**
 * @brief Append jump run to krle string
 */
void krle_packJumpRun(KaelTree *krleFormat, uint8_t currentMode, uint32_t *jumpLength, uint32_t maxJump){
	#if KRLE_EXTRA_DEBUGGING==1
		printf("Jump runs ");
	#endif
	while(*jumpLength){
		uint8_t runLength = kaelMath_min(*jumpLength, maxJump);
		if(currentMode==KRLE_PIXEL_RUN){
			kaelTree_push(krleFormat,&(uint8_t){KRLE_RUN_JUMP});
		}else
		if(currentMode==KRLE_PIXEL_PAIR){
			kaelTree_push(krleFormat,&(uint8_t){KRLE_PAIR_JUMP});
		}
		kaelTree_push(krleFormat,&(uint8_t){runLength});
		
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
void krle_packPixelRun(KaelTree *krleFormat, uint8_t *currentMode, uint8_t paletteIndex, uint32_t *pixelLength, uint32_t maxPixelLength){
	krle_switchPixelMode(krleFormat, currentMode, KRLE_PIXEL_RUN);
	//Chain of same pixels ended
	#if KRLE_EXTRA_DEBUGGING==1
		printf("palette %u runs ", paletteIndex);
	#endif

	while(*pixelLength){
		uint8_t runLength = kaelMath_min(*pixelLength, maxPixelLength);
		uint8_t pixelByte = kaelMath_u8pack(paletteIndex, runLength);
		kaelTree_push(krleFormat,&(uint8_t){pixelByte});

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

void krle_packPixelPair(KaelTree *krleFormat, uint8_t *currentMode, uint8_t first, uint8_t second ){
	krle_switchPixelMode(krleFormat, currentMode, KRLE_PIXEL_PAIR);
	#if KRLE_EXTRA_DEBUGGING==1
		printf("Pair %u -> %u\n", first, second);
	#endif
	uint8_t byte = kaelMath_u8pack(first, second); //High nibble is read first, little endian
	kaelTree_push(krleFormat,&(uint8_t){byte});
}

void krle_unpackPixelPair(uint8_t byte, uint8_t *first, uint8_t *second ){
	kaelMath_u8unpack(byte, first, second); //High nibble is read first
}


//------ RLE Detection Incrementor ------

uint8_t krle_isWithinRange(uint16_t height, uint32_t *y){
	return *y < height;
}

/**
 * @brief Portions of the KRLE runs have to be incremented while nested
 */
uint8_t krle_incrementor(uint16_t width, uint16_t height, uint32_t *i, uint32_t *x, uint32_t *y, uint32_t addend){
	if(i==NULL || x==NULL || y==NULL ){
		return 0xFF;
	}
	(*x)+=1;
	if(*x>=width){
		*y+=addend;
		*x=0;
	}
	*i = *y * width + *x;

	return krle_isWithinRange(height, y);
}

/**
 * @brief encode TGA pixels into KRLE
 * 
 * @note Formatting
 * Call marker once 			[pixelRuns : 8bit] [color : 4bit, length 4bit] -||- ...
 * Call marker once 			[pixelPair : 8bit] [color : 4bit, color  4bit] -||- ...
 * Call marker every jump 	[KRLE_RUN_JUMP : 8bit] [length : 8bit] ...
 */
void krle_pixelsToKRLE(KaelTree *krleFormat, Krle_LAB labPalette[16], uint8_t *pixels, uint16_t width, uint16_t height, uint8_t stretchFactor){
	if(NULL_CHECK(krleFormat) || NULL_CHECK(labPalette) || NULL_CHECK(pixels) ){
		return;
	}
	stretchFactor = stretchFactor==0 ? 1 : stretchFactor;

	//Raw 5 bit colors per pixel (4bit colors 1bit alpha), 1.6 pixels in byte
	//Converted fromat fits 3-2 pixels into one byte. Worst case scenario ~8 bits in one byte  
	kaelTree_alloc(krleFormat,sizeof(uint8_t));
	kaelTree_reserve(krleFormat, width * height / (2*stretchFactor));
	
	//Worst case scenario prints pair/run marker too often, if switching to runs, stay there for cooldownLength pixels
	//Run mode marker has to be used for single pixels anyways
	//Pairs do well in noise or dithering where runs are longer than 1 pixels
	uint8_t cooldownLength = 15;
	uint8_t startCooldown = 1;
	uint8_t modeCooldown = 0;

	const uint32_t maxJump=UINT8_MAX;
	const uint32_t maxPixelLength=15;
	const uint8_t alphaClip=128;

	uint32_t pixelCount = 0; //for debugging
	uint32_t pixelLength=0; //How long pixel run
	uint8_t prevPalette = 0xFF;
	uint8_t currentPixelMode = KRLE_PIXEL_RUN; //is current mode runs or pairs?

	uint32_t i=0,x=0,y=0;

	//byte order: BGRA
	while( krle_isWithinRange(height, &y) ){

		countJumpRuns: //jump runs
		if( pixels[i*4+3] < alphaClip ){
			uint32_t jumpLength=0;
			do{
				uint8_t alpha = pixels[i*4+3];
				if(alpha<alphaClip){
					jumpLength++;
				}
				if(alpha>alphaClip && jumpLength){
					pixelCount+=jumpLength;
					krle_packJumpRun(krleFormat, currentPixelMode, &jumpLength, maxJump);
					break;
				}
			}while( krle_incrementor(width, height, &i, &x, &y, stretchFactor) );
		}

		if(!krle_isWithinRange(height, &y)){
			break;
		}

		prevPalette=0xFF;
		pixelLength = 0;
		uint32_t runLength=0;
		modeCooldown = startCooldown;
		do{
			//pixel runs
			uint8_t alpha = pixels[i*4+3];

			uint8_t thisPalette = 0xFF;
			if(alpha>alphaClip){
				Krle_RGB rgbTriple;
				rgbTriple.r = pixels[i*4+2];
				rgbTriple.g = pixels[i*4+1];
				rgbTriple.b = pixels[i*4+0];

				thisPalette = krle_palettizeRGB(labPalette, rgbTriple);
				#if KRLE_EXTRA_DEBUGGING==1
					krle_debugColorDistance(rgbTriple, krle_orchisPalette, thisPalette);
				#endif

				if( thisPalette==prevPalette ){
					pixelLength++;
				}else{
					runLength = pixelLength;
					pixelLength = 1;
				}
			}

			if(runLength){
				if((runLength>1 || modeCooldown) && thisPalette != 0xFF){
					if(currentPixelMode!=KRLE_PIXEL_RUN){
						//recently switched. Prevent switching back to pairs immediately
						modeCooldown = cooldownLength;
					}
					modeCooldown -= modeCooldown!=0;

					//run longer than 1 ended. 
					pixelCount+=runLength;
					krle_packPixelRun(krleFormat, &currentPixelMode, prevPalette, &runLength, maxPixelLength);
				}
				else
				if(runLength<=1 && thisPalette != 0xFF && prevPalette!=0xFF ){
					//pixel pair (even if latter was at beginning of a run)
					pixelCount+=2;
					krle_packPixelPair(krleFormat, &currentPixelMode, prevPalette, thisPalette); //Order is important first, second'
					thisPalette=0xFF;
					prevPalette=0xFF;
				}
				runLength=0;
			}

			if( alpha<alphaClip ){
				if(pixelLength && prevPalette!=0xFF){
					//clear remaining pixels before jump
					pixelCount+=pixelLength;
					krle_packPixelRun(krleFormat, &currentPixelMode, prevPalette, &pixelLength, maxPixelLength);
					pixelLength=0;
				}
				//Jump without increment as the same byte will be read by jump runs
				goto countJumpRuns;
			}

			prevPalette = thisPalette;

		}while( krle_incrementor(width, height, &i, &x, &y, stretchFactor) );
	}

	//clear remaining pixels. Trailing jumps are ignored
	if(prevPalette!=0xFF && pixelLength){
		pixelCount+=pixelLength;
		krle_packPixelRun(krleFormat, &currentPixelMode, prevPalette, &pixelLength, maxPixelLength);
	}
	//May differ from actual size since trailing jumps are ignored
	printf("Wrote %d pixels into KRLE string\n",pixelCount);

	//Null terminate
	kaelTree_push(krleFormat,&(uint8_t){'\0'});
}





//------ KRLE to 32bit BGRA pixels ------

uint8_t krle_jumpToPixels(uint8_t *TGAPixels, Krle_header header, uint8_t jumpLength, uint32_t *px){
	if(jumpLength==0){
		printf("Pixel %u Invalid Jump Length\n", *px);
		return KRLE_ERR_ZERO_JUMP;
	}
	for(uint16_t i=0; i<jumpLength; i++){
		//Transparent white
		TGAPixels[*px*4+2] = 255; //R
		TGAPixels[*px*4+1] = 255; //G
		TGAPixels[*px*4+0] = 255; //B
		TGAPixels[*px*4+3] = 0; //A
		*px+=1;
		if(*px >= header.width * header.height){
			return KRLE_ERR_PIXEL_OVERFLOW;
		}
	}
	return KRLE_SUCCESS;
}



//--- run and pair nibbles ---

uint8_t krle_runToPixels(uint8_t *TGAPixels, Krle_header header, uint8_t byte, uint32_t *px){
	uint8_t paletteIndex;
	uint8_t length;
	
	krle_unpackPixelRun(byte, &paletteIndex, &length);
	if(length==0){
		printf("Pixel %u Invalid Run Length\n", *px);
		return KRLE_ERR_ZERO_JUMP;
	}
	for(uint16_t i=0; i<length; i++){
		TGAPixels[*px*4+2] = krle_orchisPalette[paletteIndex][0]; //R
		TGAPixels[*px*4+1] = krle_orchisPalette[paletteIndex][1]; //G
		TGAPixels[*px*4+0] = krle_orchisPalette[paletteIndex][2]; //B
		TGAPixels[*px*4+3] = 255; //A
		*px+=1;

		if(*px >= header.width * header.height){
			return KRLE_ERR_PIXEL_OVERFLOW;
		}
	}
	return KRLE_SUCCESS;
}

uint8_t krle_pairToPixels(uint8_t *TGAPixels, Krle_header header, uint8_t byte, uint32_t *px){
		uint8_t firstPalette;
		uint8_t secondPalette;
		
		krle_unpackPixelPair(byte, &firstPalette, &secondPalette);
		
		if(firstPalette == secondPalette){
			return KRLE_IDENTICAL_PAIR;
		}
		
		TGAPixels[*px*4+2] = krle_orchisPalette[firstPalette][0]; //R
		TGAPixels[*px*4+1] = krle_orchisPalette[firstPalette][1]; //G
		TGAPixels[*px*4+0] = krle_orchisPalette[firstPalette][2]; //B
		TGAPixels[*px*4+3] = 255; //A
		*px+=1;

		if(*px >= header.width * header.height){
			return KRLE_ERR_PIXEL_OVERFLOW;
		}
		
		TGAPixels[*px*4+2] = krle_orchisPalette[secondPalette][0]; //R
		TGAPixels[*px*4+1] = krle_orchisPalette[secondPalette][1]; //G
		TGAPixels[*px*4+0] = krle_orchisPalette[secondPalette][2]; //B
		TGAPixels[*px*4+3] = 255; //A
		*px+=1;

		return KRLE_SUCCESS;
}



/**
 * @brief Convert KRLE null terminated byte string into 32-bit BGRA pixels
 */
uint32_t krle_KRLEToPixels(const uint8_t *KRLEString, uint8_t *TGAPixels, const Krle_header header){
	if(NULL_CHECK(KRLEString) || NULL_CHECK(TGAPixels)){
		printf("NULL input in krle_KRLEToPixels()\n");
		return 0;
	}
	uint32_t totalPixels = header.width*header.height;
	uint32_t px=0; //pixel index

	uint8_t currentMode = KRLE_PIXEL_RUN;

	const uint8_t *readHead = KRLEString;

	uint8_t code=KRLE_SUCCESS;

	while( px<totalPixels && readHead[0]!=0){
		if(currentMode==KRLE_PIXEL_RUN){
			switch(readHead[0]){
				case KRLE_PIXEL_PAIR:
					currentMode = KRLE_PIXEL_PAIR;
					break;
	
				case KRLE_RUN_JUMP:
					readHead++; //Skip marker
					code = krle_jumpToPixels(TGAPixels, header, readHead[0], &px);
					break;
	
				default:
					code = krle_runToPixels(TGAPixels, header, readHead[0], &px);
					break;
			}
		}else 
		if(currentMode==KRLE_PIXEL_PAIR){
			//Marker values switch in modes
			switch(readHead[0]){
				case KRLE_PIXEL_RUN:
					currentMode = KRLE_PIXEL_RUN;
					break;
	
					case KRLE_PAIR_JUMP:
						readHead++; //Skip marker
						code = krle_jumpToPixels(TGAPixels, header, readHead[0], &px);
						break;
	
				default:
					code = krle_pairToPixels(TGAPixels, header, readHead[0], &px);
					break;
			}
		}
		
		if(code!=KRLE_SUCCESS){
			//Illegal things by KRLE standards
			if(code==KRLE_ERR_ZERO_JUMP){
				printf("Zero jump is not allowed");
			}else
			if(code==KRLE_ERR_PIXEL_OVERFLOW){
				printf("Read too many pixels from KRLE string");
			}
			if(code==KRLE_IDENTICAL_PAIR){
				printf("2 identical nibbles in PAIR mode are used as markers");
			}
			break;
		}
		readHead++;
	}

	//TODO: apply stetch factor

	printf("Read %u pixels from KRLE string\n",px);
	return px;
}



