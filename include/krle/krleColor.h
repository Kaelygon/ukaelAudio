/**
 * @file krleBase.c
 * 
 * @brief Header, KRLE Color conversion functions
 * 
 */
#pragma once

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "kaelygon/global/kaelMacros.h"

#include "krle/krleBase.h"

typedef struct{
	uint8_t r, g, b;
}Krle_RGB;

typedef struct{
	float l, a, b;
}Krle_LAB;

extern const Krle_LAB KRLE_MAGENTA_LAB; //Debug color
extern const uint16_t KRLE_PALETTE_SIZE;

//Default palette
extern const Krle_RGB krle_orchisPalette[16];
void krle_paletteRGBToLAB(const Krle_RGB *inputRGB, Krle_LAB *outputLAB, const uint16_t length);

//LAB <-> RGB
Krle_LAB krle_RGBToLAB(Krle_RGB rgbPalette);
Krle_RGB krle_LABToRGB( Krle_LAB LABCol);

//--- Conversion helpers ---
float krle_labDistance(const Krle_LAB lab1, const Krle_LAB lab2);
float krle_rgbDistance(const Krle_RGB rgb1, const Krle_RGB rgb2);
int krle_palettizeLAB(const Krle_LAB *labPalette, const Krle_LAB labColor, uint16_t const length);
void unit_krle_printRGB24Pixels(const uint8_t *TGAPixels, const uint32_t pixelsTotal);

//--- Row sampling ---

Krle_LAB krle_nearestRow   (const  uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor);
Krle_LAB krle_LABAvgRow    (const  uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor);
Krle_LAB krle_bilinearRow  (const  uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor);

