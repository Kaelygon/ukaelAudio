/**
 * @file krle.h
 * 
 * @brief header, TGA to KRLE (kael_ run length encoding) conversion and paletize 
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

#ifndef KRLE_EXTRA_DEBUGGING
	#define KRLE_EXTRA_DEBUGGING 0
#endif

typedef enum{
	KRLE_SUCCESS				= 0,
	KRLE_ERR_ZERO_JUMP		= 128,
	KRLE_ERR_PIXEL_OVERFLOW = 129,
	KRLE_IDENTICAL_PAIR 		= 130,
}krle_error;

/*
 * Call marker once 			[pixelRuns : 8bit] [color : 4bit, length 4bit] -||- ...
 * Call marker once 			[pixelPair : 8bit] [color : 4bit, color  4bit] -||- ...
 * Call marker every jump 	[pixelJump : 8bit] [length : 8bit] ...
 * 
 * Markers switch depending if you are in pixelModeRun or pixelModePair
 * This is result of free binary ranges changing depending how the two nibbles are constructed  
 * For example calling pixelModeRun in run mode will results in 1 red pixel
 * 
 * byte nibbles = [hi : 4bit, lo: 4bit]
*/
typedef enum{
	pixelTerminate = 0, //NULL termination character

	// --- pixel run mode ---
	//@note In run mode there can never be
	// byte format = [colorONE : 4bit, length : 4bit]

	pixelModePair	= 1<<4u, //TODO: Switch mode, 
	pixelRunJump	= 2<<4u, //Advance by next (uint8_t)byte in run mode

	// --- Pixel pair mode ---
	//@note  in pair mode there can never be 2 of the same color so can use markers 0bABCD:ABCD
	// byte format = [colorONE : 4bit, colorTWO : 4bit]


	pixelModeRun	= 1<<4U | 1, //Return to run mode
	pixelPairJump  = 2<<4u | 2, //jump in pair mode

}Krle_pixelMarker;

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
}Krle_TGAHeader;
#pragma pack(pop)

typedef struct {
	uint8_t r, g, b;
}Krle_RGB;

typedef struct {
	float l, a, b;
}Krle_LAB;


extern uint8_t krle_orchisPalette[16][3];

Krle_TGAHeader krle_readTGAFile(const char *filePath, uint8_t **pixelsTGA);

uint8_t krle_jumpToPixels(uint8_t *pixelsTGA, Krle_TGAHeader header, uint8_t jumpLength, uint32_t *px);
uint8_t krle_runNibbleToPixels(uint8_t *pixelsTGA, Krle_TGAHeader header, uint8_t byte, uint32_t *px);
uint8_t krle_pairNibbleToPixels(uint8_t *pixelsTGA, Krle_TGAHeader header, uint8_t byte, uint32_t *px);

uint32_t krle_toPixels(const uint8_t *krleString, uint8_t *pixelsTGA, const Krle_TGAHeader header);
void krle_pixelsToKRLE(KaelTree *krleFormat, Krle_LAB labPalette[16], Krle_TGAHeader header, uint8_t *pixels, uint8_t stretchFactor);
void krle_initLabPalette(Krle_LAB *labPalette, uint8_t rgbPalette[16][3]);