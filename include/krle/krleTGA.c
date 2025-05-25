/**
 * @file krleTGA.c
 * 
 * @brief header, Conversion tools to convert between TGA and KRLE (Kael Run Length Encoding)
 * 
 * Optimized for printing decoding bytes into ANSI escape codes rather than file size
 */

#include "krle/krleTGA.h"

Krle_LAB KRLE_MAGENTA_LAB = {.l=60.3, .a=98.3, .b=-60.8}; //Magenta

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
	Krle_LAB orchisPaletteLAB[16] = {0};
	krle_paletteRGBToLAB(orchisPaletteLAB, krle_orchisPalette);
	

	//Convert TGA to KRLE string
	KaelTree krleFormat = {0};
	krle_pixelsToKRLE(&krleFormat, orchisPaletteLAB, TGAPixels, TGAHeader.width, TGAHeader.height, stretchFactor, sampleType);
	free(TGAPixels);

	uint16_t squashedHeight = (TGAHeader.height+(stretchFactor-1))/stretchFactor; //ceil
	Krle_header KRLEHeader = krle_createKRLEHeader( TGAHeader.width, squashedHeight, kaelTree_length(&krleFormat), stretchFactor);

	const uint8_t *KRLEString = kaelTree_get(&krleFormat, 0);
	krle_writeKRLEFile(KRLEString, KRLEHeader, KRLEFile);
	kaelTree_free(&krleFormat);


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
Krle_LAB krle_RGBToLAB(Krle_RGB rgbPalette) {
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
	Krle_LAB lab1 = krle_RGBToLAB(rgb1);
	Krle_LAB lab2 = krle_RGBToLAB(rgb2);
	return krle_labDistance(lab1, lab2);
}

//--- LAB to RGB ---

// https://web.archive.org/web/20111111080001/http://www.easyrgb.com/index.php?X=MATH&H=01#tex1
Krle_RGB krle_LABToRGB( Krle_LAB LABCol){
	float var_Y = ( LABCol.l + 16. ) / 116.;
	float var_X = LABCol.a / 500. + var_Y;
	float var_Z = var_Y - LABCol.b / 200.;

	if ( pow(var_Y,3) > 0.008856 ) var_Y = pow(var_Y,3);
	else                      var_Y = ( var_Y - 16. / 116. ) / 7.787;
	if ( pow(var_X,3) > 0.008856 ) var_X = pow(var_X,3);
	else                      var_X = ( var_X - 16. / 116. ) / 7.787;
	if ( pow(var_Z,3) > 0.008856 ) var_Z = pow(var_Z,3);
	else                      var_Z = ( var_Z - 16. / 116. ) / 7.787;

	float X = 95.047 * var_X ;    //ref_X =  95.047     Observer= 2°, Illuminant= D65
	float Y = 100.000 * var_Y  ;   //ref_Y = 100.000
	float Z = 108.883 * var_Z ;    //ref_Z = 108.883


	var_X = X / 100. ;       //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
	var_Y = Y / 100. ;       //Y from 0 to 100.000
	var_Z = Z / 100. ;      //Z from 0 to 108.883

	float var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
	float var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
	float var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

	if ( var_R > 0.0031308 ) var_R = 1.055 * pow(var_R , ( 1 / 2.4 ))  - 0.055;
	else                     var_R = 12.92 * var_R;
	if ( var_G > 0.0031308 ) var_G = 1.055 * pow(var_G , ( 1 / 2.4 ) )  - 0.055;
	else                     var_G = 12.92 * var_G;
	if ( var_B > 0.0031308 ) var_B = 1.055 * pow( var_B , ( 1 / 2.4 ) ) - 0.055;
	else                     var_B = 12.92 * var_B;

	Krle_RGB RGBColor = {
		var_R * 255.,
		var_G * 255.,
		var_B * 255.
	};
	return RGBColor;
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
		labPalette[i] = krle_RGBToLAB(rgbTriple);
	}
}

/**
 * @brief Find nearest color to RGB triple
 */
int krle_palettizeLAB(Krle_LAB *labPalette, Krle_LAB labColor) {
	
	float minDist = INFINITY;
	int minIndex = 0;

	for (int i = 0; i < 16; i++) {
		 float dist = krle_labDistance(labColor, labPalette[i]);

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
 * @brief Append jump run to krle string
 */
void krle_packJumpRun(KaelTree *krleFormat, uint32_t *jumpLength, uint32_t maxJump){
	#if KRLE_EXTRA_DEBUGGING==1
		printf("Jump runs ");
	#endif
	while(*jumpLength){
		uint8_t runLength = kaelMath_min(*jumpLength, maxJump);
		kaelTree_push(krleFormat,&(uint8_t){KRLE_PIXEL_JUMP});
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
void krle_packPixelRun(KaelTree *krleFormat, uint8_t paletteIndex, uint32_t *pixelLength, uint32_t maxPixelLength){
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


//------ RLE Detection Incrementor ------

/**
 * @brief Validate row offset and return pixel offset to a row in the same column. If invalid return 0. If px is invalid, return UINT32_MAX
 * 
 * Sometimes interpolation results choosing pixel beyond last row if stretchFactor doesn't divide height.
 */
uint32_t krle_findFirstValidRow(uint8_t rowOffset, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px){
	uint32_t flatOffset = rowOffset * TGAWidth; // px + flatOffset == pixel in same column below of current pixel

	//Validate offset and find first valid pixel
	if(px + flatOffset >= TGAWidth * TGAHeight){
		if(rowOffset==0){
			return UINT32_MAX; //px is invalid
		}
		//Find first valid pixel
		do{
			rowOffset--;
			if(rowOffset==0){
				//Only target row is valid;
				return 0;
			}
			flatOffset = rowOffset * TGAWidth;
		}while(px + flatOffset >= TGAWidth * TGAHeight);
	}
	
	return flatOffset;
}

/**
 * @brief Sample RGB row stripe nearest neighbor, take middle pixle of the rwo
 */
Krle_LAB krle_nearestRow(uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
	if(NULL_CHECK(TGAPixels)){
		return KRLE_MAGENTA_LAB;
	}

	uint32_t yOffset = krle_findFirstValidRow(((stretchFactor+1)/2), TGAWidth, TGAHeight, px);
	Krle_RGB RGBColor = (Krle_RGB){
		TGAPixels[(px+yOffset)*4+2],
		TGAPixels[(px+yOffset)*4+1],
		TGAPixels[(px+yOffset)*4+0],
	};

	return krle_RGBToLAB(RGBColor);
}

/**
 * @brief Sample RGB row stripe in LAB space average into one RGB pixel
 */
Krle_LAB krle_LABAvgRow(uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
	if(NULL_CHECK(TGAPixels)){
		return KRLE_MAGENTA_LAB;
	}
	
	Krle_LAB avgLAB = {0};
	for(uint16_t j=0; j<stretchFactor; j++){
		uint32_t yOffset = krle_findFirstValidRow(j, TGAWidth, TGAHeight, px);
		if(yOffset==UINT32_MAX){
			return KRLE_MAGENTA_LAB;
		}

		Krle_RGB RGBColor = (Krle_RGB){
			TGAPixels[(px+yOffset)*4+2],
			TGAPixels[(px+yOffset)*4+1],
			TGAPixels[(px+yOffset)*4+0],
		};

		Krle_LAB labBuf = krle_RGBToLAB(RGBColor);
		avgLAB.l += labBuf.l;
		avgLAB.a += labBuf.a;
		avgLAB.b += labBuf.b;
	}
	avgLAB.l = avgLAB.l/(float)stretchFactor;
	avgLAB.a = avgLAB.a/(float)stretchFactor;
	avgLAB.b = avgLAB.b/(float)stretchFactor;

	return avgLAB;
}

/**
 * @brief Sample RGB row stripe using bilinear interpolation in LAB space
 */
Krle_LAB krle_bilinearRow(uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
	if(NULL_CHECK(TGAPixels) || stretchFactor < 2){
		return KRLE_MAGENTA_LAB;
	}

	uint32_t offsetTop = 0;
	uint32_t offsetBot = krle_findFirstValidRow((stretchFactor-1), TGAWidth, TGAHeight, px);
	if(offsetBot==UINT32_MAX){
		return KRLE_MAGENTA_LAB;
	}

	//Top and bottom pixels
	Krle_RGB topRGB = {
		TGAPixels[(px + offsetTop) * 4 + 2],
		TGAPixels[(px + offsetTop) * 4 + 1],
		TGAPixels[(px + offsetTop) * 4 + 0]
	};
	Krle_RGB botRGB = {
		TGAPixels[(px + offsetBot) * 4 + 2],
		TGAPixels[(px + offsetBot) * 4 + 1],
		TGAPixels[(px + offsetBot) * 4 + 0]
	};
	Krle_LAB labTop = krle_RGBToLAB(topRGB);
	Krle_LAB labBot = krle_RGBToLAB(botRGB);

	float weight = 0.5;
	Krle_LAB outLAB = {
		.l = labTop.l * (1.0f - weight) + labBot.l * weight,
		.a = labTop.a * (1.0f - weight) + labBot.a * weight,
		.b = labTop.b * (1.0f - weight) + labBot.b * weight
	};

	return outLAB;
}

/**
 * @brief Is current row in canvas && fail safe
 */
uint8_t krle_isWithinRange(uint16_t width, uint16_t height, uint32_t i, uint32_t y){
	return (y < height) && (i<width*height);
}

/**
 * @brief Read each pixel in column and skip rows by stretchFactor
 */
uint8_t krle_incrementor(uint16_t width, uint16_t height, uint32_t *i, uint32_t *x, uint32_t *y, uint32_t stretchFactor){
	if(i==NULL || x==NULL || y==NULL ){
		return 0;
	}
	(*x)+=1;
	if(*x>=width){
		*y+=stretchFactor;
		*x=0;
	}
	*i = *y * width + *x;
	return krle_isWithinRange(width, height, *i, *y);
}

/**
 * @brief encode TGA (BGRA32) pixels into KRLE
 * 
 * @note Formatting
 * Call marker once 			[pixelRuns : 8bit] [color : 4bit, length 4bit] -||- ...
 * Call marker once 			[pixelPair : 8bit] [color : 4bit, color  4bit] -||- ...
 * Call marker every jump 	[KRLE_PIXEL_JUMP : 8bit] [length : 8bit] ...
 */
void krle_pixelsToKRLE(KaelTree *krleFormat, Krle_LAB labPalette[16], uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint8_t stretchFactor, uint8_t sampleType){
	if(NULL_CHECK(krleFormat) || NULL_CHECK(labPalette) || NULL_CHECK(TGAPixels) ){
		return;
	}
	stretchFactor = stretchFactor==0 ? 1 : stretchFactor;

	//Raw 5 bit colors per pixel (4bit colors 1bit alpha), 1.6 pixels in byte
	//Converted fromat fits 3-2 pixels into one byte. Worst case scenario ~8 bits in one byte  
	kaelTree_alloc(krleFormat,sizeof(uint8_t));
	kaelTree_reserve(krleFormat, TGAWidth * TGAHeight / (2*stretchFactor));

	const uint32_t maxJump=UINT8_MAX;
	const uint32_t maxPixelLength=15;
	const uint8_t alphaClip=127; //pixel is discarded at this alpha value or lower

	uint32_t pixelCount = 0; //for debugging
	uint32_t pixelLength=0; //How long pixel run
	uint8_t prevPixel = 0xFF;
	uint32_t jumpLength=0;

	uint32_t i=0,x=0,y=0;

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
	while( krle_isWithinRange(TGAWidth, TGAHeight, i, y) ){
		uint8_t alpha = TGAPixels[i*4+3];
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
				krle_packJumpRun(krleFormat, &jumpLength, maxJump);

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

				if(sampleType==KRLE_LAB_AVG){
					LABTriple = krle_LABAvgRow(TGAPixels, TGAWidth, TGAHeight, i, stretchFactor);
				}else
				if(sampleType==KRLE_BILINEAR){
					LABTriple = krle_bilinearRow(TGAPixels, TGAWidth, TGAHeight, i, stretchFactor);
				}else{
					//Default 0, KRLE_NEAREST_NEIGHBOR
					LABTriple = krle_nearestRow(TGAPixels, TGAWidth, TGAHeight, i, stretchFactor);
				}

				thisPixel = krle_palettizeLAB(labPalette, LABTriple);
				#if KRLE_EXTRA_DEBUGGING==1
					krle_debugColorDistance(LABTriple, krle_orchisPalette, thisPixel);
				#endif
			}

			if((thisPixel!=prevPixel && pixelLength) || isTransparent){
				//run longer than 1 ended OR next pixel is transparent
				pixelCount+=pixelLength;
				krle_packPixelRun(krleFormat, prevPixel, &pixelLength, maxPixelLength);
			}
			
			if(isTransparent){
				//break without increment
				isJumpRun=1;
				continue;
			}

			pixelLength++; //Add thisPixel to next run
			prevPixel = thisPixel;
		}

		krle_incrementor(TGAWidth, TGAHeight, &i, &x, &y, stretchFactor);
	}

	//clear remaining pixels. Trailing jumps are ignored
	if(prevPixel!=0xFF && pixelLength){
		pixelCount+=pixelLength;
		krle_packPixelRun(krleFormat, prevPixel, &pixelLength, maxPixelLength);
	}
	
	//May differ from actual size since trailing jumps are ignored
	printf("Wrote %d pixels into KRLE string\n",pixelCount);

	//Null terminate
	kaelTree_push(krleFormat,&(uint8_t){'\0'});
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
			krle_orchisPalette[paletteIndex][0],
			krle_orchisPalette[paletteIndex][1],
			krle_orchisPalette[paletteIndex][2],
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
 * if TGAPixels is NULL, 4*width*hegith bytes is allocated
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



