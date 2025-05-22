/**
 * @file krleCoding.c
 * 
 * @brief Implementation, TGA to KRLE (kael_ run length encoding) conversion and paletize 
 * 
 * Optimized for printing decoding bytes into ANSI escape codes rather than file size
 */

 #include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"
#include "kaelygon/book/book.h"


//{R, G, B} Tilix Orchis ANSI colors
uint8_t orchisPalette[][3] = {
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

#pragma pack(push, 1)
typedef struct {
	uint8_t idLength;
	uint8_t colorMapType;
	uint8_t dataTypeCode;
	uint16_t colorMapOrigin;
	uint16_t colorMapLength;
	uint8_t colorMapDepth;
	uint16_t xOrigin;
	uint16_t yOrigin;
	uint16_t width;
	uint16_t height;
	uint8_t bitsPerPixel;
	uint8_t imageDescriptor;
} TGAHeader;
#pragma pack(pop)




typedef struct {
	uint8_t r, g, b;
}KrleRGB;

typedef struct {
	float l, a, b;
}KrleLAB;

static float krle_pivotRGB(float n) {
	n /= 255.0f;
	return (n > 0.04045f) ? powf((n + 0.055f) / 1.055f, 2.4f) : (n / 12.92f);
}

static float krle_pivotXYZ(float n) {
	return (n > 0.008856f) ? powf(n, 1.0f/3.0f) : (7.787f * n) + (16.0f / 116.0f);
}

KrleLAB krle_rgbToLab(KrleRGB rgbPalette) {
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

	KrleLAB lab;
	lab.l = (116.0f * Y) - 16.0f;
	lab.a = 500.0f * (X - Y);
	lab.b = 200.0f * (Y - Z);
	return lab;
}

void krle_initLabPalette(KrleLAB *labPalette, uint8_t rgbPalette[16][3]){
	for (int i = 0; i < 16; i++){
		KrleRGB rgbTriple = (KrleRGB){
			rgbPalette[i][0],
			rgbPalette[i][1],
			rgbPalette[i][2]
		};
		labPalette[i] = krle_rgbToLab(rgbTriple);
	}
}

float krle_labDistance(KrleLAB lab1, KrleLAB lab2){
	float dl = lab1.l - lab2.l;
	float da = lab1.a - lab2.a;
	float db = lab1.b - lab2.b;
	return dl * dl + da * da + db * db;
}

float krle_rgbDistance(KrleRGB rgb1, KrleRGB rgb2){
	KrleLAB lab1 = krle_rgbToLab(rgb1);
	KrleLAB lab2 = krle_rgbToLab(rgb2);
	return krle_labDistance(lab1, lab2);
}

void krle_debugColorDistance(KrleRGB rgbTriple, uint8_t palette[16][3], uint8_t index){
	KrleRGB newTriple = (KrleRGB){
		palette[index][0],
		palette[index][1],
		palette[index][2]					
	};
	float colorDistance = krle_rgbDistance(newTriple, rgbTriple);
	printf("palette %u delta %.2f\n", index, colorDistance);
}

int krle_palettizeRGB(KrleLAB *labPalette, KrleRGB rgbColor) {

	KrleLAB original = krle_rgbToLab(rgbColor);
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





//------ encode TGA to KRLE ------ 

void krle_switchPixelMode(KaelTree *krleFormat, uint8_t *currentMode, uint8_t newMode ){
	if(*currentMode==newMode){
		//No switching
		return;
	}
	switch(newMode){
		case pixelModeRun:
			*currentMode=pixelModeRun;
			kaelTree_push(krleFormat,&(uint8_t){pixelModeRun});
			break;
		case pixelModePair:
			*currentMode=pixelModePair;
			kaelTree_push(krleFormat,&(uint8_t){pixelModePair});
			break;
	}
}

void krle_printJumpRun(KaelTree *krleFormat, uint32_t *jumpLength, uint32_t maxJump){

	printf("Jump runs ");
	while(*jumpLength){
		uint8_t runLength = kaelMath_min(*jumpLength, maxJump);
		kaelTree_push(krleFormat,&(uint8_t){pixelJump});
		kaelTree_push(krleFormat,&(uint8_t){runLength});
		
		printf("%d ",runLength);
		*jumpLength -= runLength;
	}
	
	printf("\n");
}

void krle_printPixelRun(KaelTree *krleFormat, uint8_t *currentMode, uint8_t paletteIndex, uint32_t *pixelLength, uint32_t maxPixelLength){
	krle_switchPixelMode(krleFormat, currentMode, pixelModeRun);
	//Chain of same pixels ended
	printf("palette %u runs ", paletteIndex);

	while(*pixelLength){
		uint8_t runLength = kaelMath_min(*pixelLength, maxPixelLength);
		uint8_t pixelByte = kaelMath_u8pack(paletteIndex, *pixelLength);
		kaelTree_push(krleFormat,&(uint8_t){pixelByte});

		printf("%d ",runLength);
		*pixelLength -= runLength;
	}
	
	printf("\n");
}

void krle_printPair(KaelTree *krleFormat, uint8_t *currentMode, uint8_t first, uint8_t second ){
	krle_switchPixelMode(krleFormat, currentMode, pixelModePair);
	printf("Pair %u -> %u\n", first, second);
	uint8_t byte = kaelMath_u8pack(first, second); //High nibble is read first, little endian
	kaelTree_push(krleFormat,&(uint8_t){byte});
}

void krle_unpackPair(uint8_t byte, uint8_t *first, uint8_t *second ){
	kaelMath_u8unpack(byte, first, second); //High nibble is read first
}

uint8_t krle_isWithinRange(TGAHeader header, uint32_t *y){
	return *y < header.height;
}

/**
 * @brief Portions of the KRLE runs have to be incremented while nested
 */
uint8_t krle_incrementor(TGAHeader header, uint32_t *i, uint32_t *x, uint32_t *y, uint32_t addend){
	if(i==NULL || x==NULL || y==NULL ){
		return 0xFF;
	}
	(*x)+=1;
	if(*x>=header.width){
		*y+=addend;
		*x=0;
	}
	*i = *y * header.width + *x;

	return krle_isWithinRange(header, y);
}

/**
 * @brief encode TGA pixels into KRLE
 * 
 * @note Formatting
 * Call marker once 			[pixelRuns : 8bit] [color : 4bit, length 4bit] -||- ...
 * Call marker once 			[pixelPair : 8bit] [color : 4bit, color  4bit] -||- ...
 * Call marker every jump 	[pixelJump : 8bit] [length : 8bit] ...
 */
void krle_pixelsToAnsi(KaelTree *krleFormat, KrleLAB labPalette[16], TGAHeader header, uint8_t *pixels, uint8_t stretchFactor){
	if(NULL_CHECK(krleFormat) || NULL_CHECK(labPalette) || NULL_CHECK(pixels) ){
		return;
	}
	//Raw 5 bit colors per pixel (4bit colors 1bit alpha), 1.6 pixels in byte
	//Converted fromat fits 3-2 pixels into one byte. Worst case scenario ~8 bits in one byte  
	kaelTree_alloc(krleFormat,sizeof(uint8_t));
	kaelTree_reserve(krleFormat, header.width * header.height / (2*stretchFactor));
	
	//Worst case scenario prints pair/run marker too often, if switching to runs, stay there for cooldownLength pixels
	//Run mode marker has to be used for single pixels anyways
	//Pairs do well in noise or dithering where runs are longer than 1 pixels
	uint8_t cooldownLength = 15;
	uint8_t startCooldown = 1;
	uint8_t modeCooldown = 0;

	const uint32_t maxJump=UINT8_MAX;
	const uint32_t maxPixelLength=15;
	const uint8_t alphaClip=128;

	uint32_t pixelLength=0; //How long pixel run
	uint8_t prevPalette = 0xFF;
	uint8_t currentPixelMode = pixelModeRun; //is current mode runs or pairs?

	uint32_t i=0,x=0,y=0;

	//byte order: BGRA
	while( krle_isWithinRange(header, &y) ){

		countJumpRuns: //jump runs
		if( pixels[i*4+3] < alphaClip ){
			uint32_t jumpLength=0;
			do{
				uint8_t alpha = pixels[i*4+3];
				if(alpha<alphaClip){
					jumpLength++;
				}
				if(alpha>alphaClip && jumpLength){
					krle_printJumpRun(krleFormat, &jumpLength, maxJump);
					break;
				}
			}while( krle_incrementor(header, &i, &x, &y, stretchFactor) );
		}

		if(!krle_isWithinRange(header, &y)){
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
				KrleRGB rgbTriple;
				rgbTriple.r = pixels[i*4+2];
				rgbTriple.g = pixels[i*4+1];
				rgbTriple.b = pixels[i*4+0];

				thisPalette = krle_palettizeRGB(labPalette, rgbTriple);
				krle_debugColorDistance(rgbTriple, orchisPalette, thisPalette);

				if( thisPalette==prevPalette ){
					pixelLength++;
				}else{
					runLength = pixelLength;
					pixelLength = 1;
				}
			}

			if(runLength){
				if((runLength>1 || modeCooldown) && thisPalette != 0xFF){
					if(currentPixelMode!=pixelModeRun){
						//recently switched. Prevent switching back to pairs immediately
						modeCooldown = cooldownLength;
					}
					modeCooldown -= modeCooldown!=0;

					//run longer than 1 ended. 
					krle_printPixelRun(krleFormat, &currentPixelMode, prevPalette, &runLength, maxPixelLength);
				}
				else
				if(runLength<=1 && thisPalette != 0xFF && prevPalette!=0xFF ){
					//pixel pair (even if latter was at beginning of a run)
					krle_printPair(krleFormat, &currentPixelMode, prevPalette, thisPalette); //Order is important first, second'
					thisPalette=0xFF;
					prevPalette=0xFF;
				}
				runLength=0;
			}

			if( alpha<alphaClip ){
				if(pixelLength && prevPalette!=0xFF){
					//clear remaining pixels before jump
					krle_printPixelRun(krleFormat, &currentPixelMode, prevPalette, &pixelLength, maxPixelLength);
					pixelLength=0;
				}
				//Jump without increment as the same byte will be read by jump runs
				goto countJumpRuns;
			}

			prevPalette = thisPalette;

		}while( krle_incrementor(header, &i, &x, &y, stretchFactor) );
	}

	//clear remaining pixels. Trailing jumps are ignored
	if(prevPalette!=0xFF && pixelLength){
		krle_printPixelRun(krleFormat, &currentPixelMode, prevPalette, &pixelLength, maxPixelLength);
	}
}

void krle_encodeTGA(const char *filePath, uint8_t isPalette, uint8_t stretchFactor){

	FILE *file = fopen(filePath, "rb");
	if (!file) {
		perror("Failed to open file");
		return;
	}

	TGAHeader header;
	fread(&header, sizeof(TGAHeader), 1, file);

	if (header.bitsPerPixel != 32 || header.dataTypeCode != 2) {
		fprintf(stderr, "Unsupported TGA format. Only 32-bit uncompressed supported.\n");
		fclose(file);
		return;
	}

	uint32_t pixelsTotal = header.width * header.height;
	uint8_t *pixels = malloc(pixelsTotal * 4); // 4 bytes per pixel (BGRA)

	fread(pixels, 4, pixelsTotal, file);
	fclose(file);

	//Print RGB triples
	if(isPalette){
		for(int i=0; i<(int)pixelsTotal; i++){
			uint8_t b = pixels[i*4+0];
			uint8_t g = pixels[i*4+1];
			uint8_t r = pixels[i*4+2];
			printf("{%u, %u, %u},\n",r,g,b);
		}
		if(pixelsTotal>256){
			printf("That's a pretty big palette, huh?\n");
		}
		return;
	}


	KaelTree krleFormat = {0};
	KrleLAB labPalette[16] = {0};
	krle_initLabPalette(labPalette, orchisPalette);

	krle_pixelsToAnsi(&krleFormat, labPalette, header, pixels, stretchFactor);

	free(pixels);

	uint16_t compressedSize = kaelTree_length(&krleFormat);
	uint32_t stretchedPixels = header.width*header.height/stretchFactor;
	float bytesPerPixel =  8.0*(float)compressedSize/(stretchedPixels);
	float pixelsPerByte = (float)stretchedPixels/compressedSize;
	printf("%u pixels compressed to %u bytes.\n", stretchedPixels, compressedSize);
	printf("%.2f pixels per byte\n", pixelsPerByte);
	printf("%.2f bits per pixel\n",bytesPerPixel);

	//TODO: krleFormat into file + header 

	kaelTree_free(&krleFormat);
}

int main() {
	uint8_t isPalette = 0;
	const char *filePath[] = {
		"./assets/tga/KaelTUIMerged.tga",
		"./assets/tga/orchisPalette.tga",
		"./assets/tga/noise.tga",
		"./assets/tga/worst.tga",
	}; 

	krle_encodeTGA(filePath[0], isPalette, 2);
	return 0;
}