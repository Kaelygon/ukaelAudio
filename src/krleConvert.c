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

#define KRLE_EXTRA_DEBUGGING 0

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/math/math.h"
#include "kaelygon/treeMem/tree.h"

#include "krle/krleTGA.h"

void unit_krleTGA(){
	uint8_t index = 0;

	const char *folderPath="./assets/";
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

	char TGAFile[100]={0};
	char KRLEFile[100]={0};
	snprintf( TGAFile, sizeof(TGAFile), "%stga/%s.tga", folderPath, basenameList[index] );
	snprintf( KRLEFile, sizeof(KRLEFile), "%skrle/%s.krle", folderPath, basenameList[index] );

	krle_TGAToKRLE(TGAFile, KRLEFile, ratioList[index], KRLE_BILINEAR);
	krle_KRLEToTGA(KRLEFile, "./assets/unit_KRLEToTGA.tga");
}

int main(){
	unit_krleTGA();
	return 0;
}