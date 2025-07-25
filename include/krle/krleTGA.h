/**
 * @file krleTGA.h
 * 
 * @brief header, Conversion tools to convert between TGA and KRLE (Kael Run Length Encoding)
 * 
 * Optimized for printing decoding bytes into ANSI escape codes rather than file size
 */
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "krle/krleBase.h"
#include "krle/krleColor.h"

//Row down smapleType for krle_TGAToKRLE
typedef enum{
	KRLE_NEAREST,
	KRLE_BILINEAR,
	KRLE_LAB_AVG,
}Krle_sampleType;

#ifndef KRLE_PRINT_INFO
	#define KRLE_PRINT_INFO 0
#endif
#ifndef KRLE_EXTRA_DEBUGGING
	#define KRLE_EXTRA_DEBUGGING 0
#endif

#pragma pack(push, 1)
typedef struct {
	uint8_t idLength;
	uint8_t colorMapType;
	uint8_t dataTypeCode;
	
	/* Color Map Specification */
	uint16_t colorMapOrigin;
	uint16_t colorMapLength;
	uint8_t colorMapDepth;

	/* Image Specification */
	uint16_t xOrigin;
	uint16_t yOrigin;
	uint16_t width;
	uint16_t height;
	uint8_t bitsPerPixel;
	uint8_t imageDescriptor;
}Krle_TGAHeader;
#pragma pack(pop)

//TGA<->KRLE conversion
void krle_TGAToKRLE(const char *TGAFile, const char *KRLEFile, uint8_t stretchFactor, uint8_t sampleType);
void krle_KRLEToTGA(const char *KRLEFile, const char *TGAFile);

//TGA helpers
Krle_TGAHeader krle_createTGAHeader(uint16_t width, uint16_t height);
Krle_TGAHeader krle_readTGAFile(const char *filePath, uint8_t **pixelsTGA);
void krle_writeTGAFile(const char* fileName, Krle_TGAHeader TGAHeader, uint8_t *pixelsTGA);

//KRLE helpers
Krle_header krle_createKRLEHeader(uint16_t width, uint16_t height, uint32_t length, uint8_t ratio);
Krle_header krle_readKRLEFile(const char *filePath, uint8_t **KRLEString);
void krle_writeKRLEFile(const uint8_t *krleString, Krle_header krleHeader, const char* fileName);

//KRLE<->Pixels Conversion
uint32_t krle_KRLEToPixels(const uint8_t *KRLEString, uint8_t **TGAPixels, const Krle_header header);
void krle_pixelsToKRLE(KaelTree *krleTree, const Krle_LAB *labPalette, const uint8_t *TGAPixels, uint16_t width, uint16_t height, uint8_t stretchFactor, uint8_t sampleType);








