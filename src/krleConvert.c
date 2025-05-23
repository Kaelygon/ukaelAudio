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
void unit_krle_encodeKRLE(const char *folderPath, const char *baseName, uint8_t stretchFactor){
	if(NULL_CHECK(folderPath) || NULL_CHECK(baseName)){
		return;
	}
	stretchFactor = stretchFactor==0 ? 1 : stretchFactor;
	uint8_t *TGAPixels = NULL;

	char TGAFile[100]={0};
	char krleFile[100]={0};
	snprintf( TGAFile, sizeof(TGAFile), "%stga/%s.tga", folderPath, baseName );
	snprintf( krleFile, sizeof(krleFile), "%skrle/%s.krle", folderPath, baseName );

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
	krle_pixelsToKRLE(&krleFormat, orchisPaletteLAB, TGAPixels, TGAHeader.width, TGAHeader.height, stretchFactor);

	Krle_header KRLEHeader = krle_createKRLEHeader( TGAHeader.width, TGAHeader.height, kaelTree_length(&krleFormat), stretchFactor);

	//Reuse pixel array and export TGA for manual validation (use your eye balls at the file)
	const uint8_t *KRLEString = kaelTree_get(&krleFormat, 0);
	krle_KRLEToPixels(KRLEString, TGAPixels, KRLEHeader);
	krle_writeTGAFile("./assets/unit_TGAtoKrle.tga", TGAHeader, TGAPixels);

	free(TGAPixels);

	krle_writeKRLEFile(KRLEString, KRLEHeader, krleFile);

	kaelTree_free(&krleFormat);


	//info printing
	uint16_t compressedSize = KRLEHeader.length;
	uint32_t stretchedPixels = TGAHeader.width*TGAHeader.height/stretchFactor;
	float bytesPerPixel =  8.0*(float)compressedSize/(stretchedPixels);
	float pixelsPerByte = (float)stretchedPixels/compressedSize;
	printf("%u pixels converted to %u bytes\n", stretchedPixels, compressedSize);
	printf("%.2f pixels per byte\n", pixelsPerByte);
	printf("%.2f bits per pixel\n",bytesPerPixel);
}






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
 * @brief KRLE to TGA conversion test function
 */
void unit_krle_decodeKRLE(const char *folderPath, const char *baseName){

	//char TGAFile[100]={0};
	char krleFile[100]={0};
	//snprintf( TGAFile, sizeof(TGAFile), "%stga/%s.tga", folderPath, baseName );
	snprintf( krleFile, sizeof(krleFile), "%skrle/%s.krle", folderPath, baseName );

	uint8_t *KRLEString = NULL;
	Krle_header KRLEHeader = krle_readKRLEFile(krleFile, &KRLEString);
	
	uint8_t *TGAPixels = calloc( 4*KRLEHeader.height*KRLEHeader.width, sizeof(uint8_t)); //32bits per pixel
	if(TGAPixels==NULL){
		printf("TGAPixels failed to alloc\n");
		return;
	}

	krle_KRLEToPixels(KRLEString, TGAPixels, KRLEHeader);
	Krle_TGAHeader TGAHeader = krle_createTGAHeader(KRLEHeader.height, KRLEHeader.width);
	krle_writeTGAFile("./assets/unit_KRLEToTGA.tga", TGAHeader, TGAPixels);

	free(TGAPixels);

	free(KRLEString);
}


int main(){

	const char *floderPath="./assets/";
	const char *basenameList[] = {
		"kaelTui",
		"orchisPalette",
		"noise",
		"worst",
	}; 

	unit_krle_encodeKRLE(floderPath, basenameList[0], 0);

	unit_krle_decodeKRLE(floderPath, basenameList[0]);

	return 0;
}