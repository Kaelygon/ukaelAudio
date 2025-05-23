/**
 * @file krleConvert.c
 * 
 * @brief Program, Test program and tool to convert between TGA and KRLE
 * 
 * TODO: Create standard for KRLE header
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define KRLE_EXTRA_DEBUGGING 0

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"
#include "kaelygon/book/book.h"

#include "krle/krleTGA.h"



/**
 * @brief TGA to krle encode test function
 */
void krle_encodeKRLE(const char *filePath, uint8_t isPalette, uint8_t stretchFactor){
	uint8_t *pixelsTGA = NULL;

	Krle_TGAHeader header = krle_readTGAFile(filePath, &pixelsTGA);
	uint32_t pixelsTotal = header.width * header.height;

	//Print RGB triples
	if(isPalette){
		for(uint32_t i=0; i<pixelsTotal; i++){
			uint8_t b = pixelsTGA[i*4+0];
			uint8_t g = pixelsTGA[i*4+1];
			uint8_t r = pixelsTGA[i*4+2];
			printf("{%u, %u, %u},\n",r,g,b);
		}
		if(pixelsTotal>256){
			printf("That's a pretty big palette, huh?\n");
		}
		return;
	}


	//Convert TGA to KRLE string
	KaelTree krleFormat = {0};
	Krle_LAB labPalette[16] = {0};
	krle_initLabPalette(labPalette, krle_orchisPalette);

	krle_pixelsToKRLE(&krleFormat, labPalette, header, pixelsTGA, stretchFactor);



	//Reuse pixel array and export TGA for debugging

	const uint8_t *krleString = kaelTree_get(&krleFormat, 0);
	krle_toPixels(krleString, pixelsTGA, header);
	
	//Write to file
	const char *palettizedFile = "./assets/krlePalettized.tga";
	FILE *outFile = fopen(palettizedFile, "wb");
	if(outFile){
		fwrite(&header, sizeof(Krle_TGAHeader), 1, outFile);
		fwrite(pixelsTGA, 4*pixelsTotal*sizeof(uint8_t), 1, outFile); //4 bytes per pixel
		fclose(outFile);
	}else{
		perror("Failed to open outFile");
	}

	free(pixelsTGA);



	//info printing
	uint16_t compressedSize = kaelTree_length(&krleFormat);
	uint32_t stretchedPixels = header.width*header.height/stretchFactor;
	float bytesPerPixel =  8.0*(float)compressedSize/(stretchedPixels);
	float pixelsPerByte = (float)stretchedPixels/compressedSize;
	printf("%u pixels converted to %u bytes\n", stretchedPixels, compressedSize);
	printf("%.2f pixels per byte\n", pixelsPerByte);
	printf("%.2f bits per pixel\n",bytesPerPixel);

	//TODO: krleFormat into file + header 

	kaelTree_free(&krleFormat);
}

int main() {
	uint8_t isPalette = 0;
	const char *filePath[] = {
		"./assets/tga/kaelTui.tga",
		"./assets/tga/orchisPalette.tga",
		"./assets/tga/noise.tga",
		"./assets/tga/worst.tga",
	}; 

	krle_encodeKRLE(filePath[0], isPalette, 1);
	return 0;
}