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
#include <math.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "krle/krleBase.h"

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

typedef struct {
	uint8_t r, g, b;
}Krle_RGB;

typedef struct {
	float l, a, b;
}Krle_LAB;




extern uint8_t krle_orchisPalette[16][3];
void krle_paletteRGBToLAB(Krle_LAB *labPalette, uint8_t rgbPalette[16][3]);

//TGA
Krle_TGAHeader krle_createTGAHeader(uint16_t width, uint16_t height);
Krle_TGAHeader krle_readTGAFile(const char *filePath, uint8_t **pixelsTGA);
void krle_writeTGAFile(const char* fileName, Krle_TGAHeader TGAHeader, uint8_t *pixelsTGA);

//KRLE
Krle_header krle_createKRLEHeader(uint16_t width, uint16_t height, uint32_t length, uint8_t ratio);
void krle_writeKRLEFile(const uint8_t *krleString, Krle_header krleHeader, const char* fileName);

//KRLE<->Pixels
uint32_t krle_KRLEToPixels(const uint8_t *krleString, uint8_t *pixelsTGA, const Krle_header header);
void krle_pixelsToKRLE(KaelTree *krleFormat, Krle_LAB labPalette[16], uint8_t *pixels, uint16_t width, uint16_t height, uint8_t stretchFactor);








