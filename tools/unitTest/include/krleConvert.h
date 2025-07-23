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

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "krle/krleTGA.h"

//https://stackoverflow.com/questions/60570777/compare-two-binary-files-in-c
int krleTGA_unit_compareFiles(const char *path1, const char *path2) {

	FILE *fp1 = fopen(path1, "r");
	FILE *fp2 = fopen(path2, "r");
	unsigned long pos;
	int c1, c2;
	for (pos = 0;; pos++) {
		c1 = getc(fp1);
		c2 = getc(fp2);
		if (c1 != c2 || c1 == EOF)
				break;
	}
	if (c1 == c2) {
		printf("Success! %lu bytes\n", pos);
		return 0;  // files are identical
	} else
	if (c1 == EOF) {
		printf("FAIL! file1 is included in file2, the first %lu bytes are identical\n", pos);
		return 1;
	} else
	if (c2 == EOF) {
		printf("FAIL! file2 is included in file1, the first %lu bytes are identical\n", pos);
		return 2;
	} else {
		printf("FAIL! file1 and file2 differ at position %lu: 0x%02X <> 0x%02X\n", pos, c1, c2);
		return 3;
	}
}

/**
 * @brief Convert TGA->KRLE->TGA file twice iteratively. The result of both passes should be the same
 */
void krleTGA_unit(){
	

	const char *folderPath="./assets/krleUnitTest";
	const char *firstPass="./assets/krleUnitTest/firstPass";
	const char *secondPass="./assets/krleUnitTest/secondPass";

	const char *basenameList[] = {
		"kaelTui",
		"orchisPalette",
		"noise",
		"worst",
		"tienaNMS",
	}; 
	const uint8_t ratioList[] = {
		2,
		1,
		3,
		4,
		2,
	};
	uint8_t imgCount = sizeof(basenameList)/sizeof(basenameList[0]);

	for(uint8_t index=0; index<(imgCount-1); index++){
		char TGAFile[100] ={0};
		char KRLEFile[100]={0};
		char firstPassFile[100]={0};
		char secondPassFile[100]={0};
		snprintf( TGAFile, sizeof(TGAFile), "%s/tga/%s.tga", folderPath, basenameList[index] );
		snprintf( KRLEFile, sizeof(KRLEFile), "%s/krle/%s.krle", folderPath, basenameList[index] );
		snprintf( firstPassFile, sizeof(firstPassFile), "%s/%s.tga", firstPass, basenameList[index] );
		snprintf( secondPassFile, sizeof(firstPassFile), "%s/%s.tga", secondPass, basenameList[index] );

		//First pass
		krle_TGAToKRLE(TGAFile, KRLEFile, ratioList[index], KRLE_BILINEAR);
		krle_KRLEToTGA(KRLEFile, firstPassFile);

		//Second pass
		krle_TGAToKRLE(firstPassFile, KRLEFile, ratioList[index], KRLE_BILINEAR);
		krle_KRLEToTGA(KRLEFile, secondPassFile);

		printf("Converted %s.tga TGA->KRLE->TGA.\n", basenameList[index] );	
		krleTGA_unit_compareFiles((const char*)firstPassFile, (const char*)secondPassFile);
		printf("\n");
	}


	printf("unitTest_krleTGA Done\n");	
}