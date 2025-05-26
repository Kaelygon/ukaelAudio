/**
 * @file krleBase.c
 * 
 * @brief Implementation, KRLE standard minimal definitions 
 * 
 */

#include "krle/krleBase.h"



//------ Global variables ------



/**
 * @brief SGR color standard
 * 8x Dim FG], [8x Dim BG], [8x Bright FG], [8x Bright BG]
 */
const uint8_t krle_defaultColorTable[32] = {
	ANSI_FG_BLACK 				, //0
	ANSI_FG_RED 				,
	ANSI_FG_GREEN	 			,
	ANSI_FG_YELLOW 			,
	ANSI_FG_BLUE 				,
	ANSI_FG_MAGENTA 			,
	ANSI_FG_CYAN 				,
	ANSI_FG_WHITE	 			,

	ANSI_BG_BLACK 				, //0b1000
	ANSI_BG_RED 				,
	ANSI_BG_GREEN 				,
	ANSI_BG_YELLOW 			,
	ANSI_BG_BLUE 				,
	ANSI_BG_MAGENTA 			,
	ANSI_BG_CYAN 				,
	ANSI_BG_WHITE 				,

	ANSI_FG_BRIGHT_BLACK 	, //0b10000
	ANSI_FG_BRIGHT_RED 		,
	ANSI_FG_BRIGHT_GREEN	 	,
	ANSI_FG_BRIGHT_YELLOW 	,
	ANSI_FG_BRIGHT_BLUE 		,
	ANSI_FG_BRIGHT_MAGENTA 	,
	ANSI_FG_BRIGHT_CYAN 		,
	ANSI_FG_BRIGHT_WHITE	 	,

	ANSI_BG_BRIGHT_BLACK 	, //0b11000
	ANSI_BG_BRIGHT_RED 		,
	ANSI_BG_BRIGHT_GREEN 	,
	ANSI_BG_BRIGHT_YELLOW 	,
	ANSI_BG_BRIGHT_BLUE 		,
	ANSI_BG_BRIGHT_MAGENTA 	,
	ANSI_BG_BRIGHT_CYAN 		,
	ANSI_BG_BRIGHT_WHITE 	,
};

/**
 * @brief SGR attribute table
 * 
 * @warning Highly recommended to have first indedx reset because at least one value of KaelTui_ansiStyle has to be non-zero. 
 * For example then reseting can be done by .back=1 .color=0 .reset=0
 */
const  uint8_t krle_defaultAttributeTable[8] = {
	ANSI_NONE, 
	ANSI_BOLD,
	ANSI_ITALIC,
	ANSI_UNDERLINE,
	ANSI_BLINK_SLOW,
	ANSI_REVERSE,
	ANSI_CROSSED_OUT,
	ANSI_FONT_1,
};


//--- Mutable global values ---

//Ptr to a color table
const uint8_t *krle_colorTable = krle_defaultColorTable;

//Ptr to an attribute table
const uint8_t *krle_attributeTable = (uint8_t *)krle_defaultAttributeTable;




