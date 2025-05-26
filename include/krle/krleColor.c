/**
 * @file krleBase.c
 * 
 * @brief Implementation, KRLE Color conversion functions
 * 
 * TODO: Move krleTGA.c color related functions here as the file is nearing 1k lines
 * 
 */

#include "krle/krleColor.h"


const Krle_LAB KRLE_MAGENTA_LAB = {.l=60.3, .a=98.3, .b=-60.8}; //Magenta for debugging
const uint16_t KRLE_PALETTE_SIZE = 16;

/**
 * @brief {R, G, B} Tilix Orchis ANSI colors
 */
const Krle_RGB krle_orchisPalette[16] = {
	{.r=  0, .g=  0, .b=  0}, //ansiBlack
	{.r=204, .g=  0, .b=  0}, //ansiRed
	{.r= 77, .g=154, .b=  5}, //ansiGreen
	{.r=195, .g=160, .b=  0}, //ansiYellow
	{.r= 52, .g=100, .b=163}, //ansiBlue
	{.r=117, .g= 79, .b=123}, //ansiMagenta
	{.r=  5, .g=151, .b=154}, //ansiCyan
	{.r=211, .g=214, .b=207}, //ansiWhite
	{.r= 84, .g= 86, .b= 82}, //ansiBrightBlack
	{.r=239, .g= 40, .b= 40}, //ansiBrightRed
	{.r=137, .g=226, .b= 52}, //ansiBrightGreen
	{.r=251, .g=232, .b= 79}, //ansiBrightYellow
	{.r=114, .g=158, .b=207}, //ansiBrightBlue
	{.r=172, .g=126, .b=168}, //ansiBrightMagenta
	{.r= 52, .g=226, .b=226}, //ansiBrightCyan
	{.r=237, .g=237, .b=235}, //ansiBrightWhite	
};

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
Krle_LAB krle_RGBToLAB(const Krle_RGB rgbPalette) {
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

float krle_labDistance(const Krle_LAB lab1, const Krle_LAB lab2){
	float dl = lab1.l - lab2.l;
	float da = lab1.a - lab2.a;
	float db = lab1.b - lab2.b;
	return dl * dl + da * da + db * db;
}

float krle_rgbDistance(const Krle_RGB rgb1, const Krle_RGB rgb2){
	Krle_LAB lab1 = krle_RGBToLAB(rgb1);
	Krle_LAB lab2 = krle_RGBToLAB(rgb2);
	return krle_labDistance(lab1, lab2);
}

//--- LAB to RGB ---

// https://web.archive.org/web/20111111080001/http://www.easyrgb.com/index.php?X=MATH&H=01#tex1
Krle_RGB krle_LABToRGB(const Krle_LAB LABCol){
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

void krle_paletteRGBToLAB(const Krle_RGB *inputRGB, Krle_LAB *outputLAB, const uint16_t length){
	for (uint16_t i = 0; i < length; i++){
		outputLAB[i] = krle_RGBToLAB(inputRGB[i]);
	}
}

/**
 * @brief Find nearest color to RGB triple
 */
int krle_palettizeLAB(const Krle_LAB *labPalette, const Krle_LAB labColor, uint16_t const length){
	float minDist = INFINITY;
	int minIndex = 0;

	for (uint16_t i=0; i<length; i++) {
		 float dist = krle_labDistance(labPalette[i], labColor);

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
void krle_debugColorDistance(Krle_RGB rgbTriple, const Krle_RGB *palette, const uint8_t index){
	Krle_RGB newTriple = (Krle_RGB){
		palette[index].r,
		palette[index].g,
		palette[index].b					
	};
	float colorDistance = krle_rgbDistance(newTriple, rgbTriple);
   printf("palette %u delta %.2f\n", index, colorDistance);
}

void unit_krle_printRGB24Pixels(const uint8_t *TGAPixels, const uint32_t pixelsTotal){
	for(uint32_t i=0; i<pixelsTotal; i++){
		uint8_t b = TGAPixels[i*4+0];
		uint8_t g = TGAPixels[i*4+1];
		uint8_t r = TGAPixels[i*4+2];
		printf("{%u, %u, %u},\n",r,g,b);
	}
	return;
}






//------ Row stripe down sampling ------

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
Krle_LAB krle_nearestRow(const uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
	if(NULL_CHECK(TGAPixels)){
		return KRLE_MAGENTA_LAB;
	}

	uint32_t yOffset=0;
	if(stretchFactor>1){
      yOffset = krle_findFirstValidRow(((stretchFactor+1)/2), TGAWidth, TGAHeight, px);
   }
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
Krle_LAB krle_LABAvgRow(const uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
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
Krle_LAB krle_bilinearRow(const uint8_t *TGAPixels, uint16_t TGAWidth, uint16_t TGAHeight, uint32_t px, uint8_t stretchFactor){
	if(NULL_CHECK(TGAPixels)){
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
