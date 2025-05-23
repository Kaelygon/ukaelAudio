/**
 * @file krleBase.c
 * 
 * @brief Implementation, KRLE standard minimal definitions 
 * 
 */

#include <stdlib.h>
#include <stdint.h>

#include "krle/krleBase.h"



//------ Global variables ------

//--- Constant values ---

/**
 * @brief List of escape sequences enumerated by KaelTui_escSeqIndex
 */
const char *krle_constChar[] = {
	"\x1b[2J",	//escSeq_clear
	"\x1b[2K",	//escSeq_clearRow
	"\x1b[0m",	//escSeq_styleReset
	"\x1b[r"		//escSeq_scrollReset
};

/**
 * @brief List of kaelTui_escSeq lengths enumerated by KaelTui_escSeqIndex
 */
const uint8_t krle_constCharLen[] = {
	4,	//escSeq_clear
	4,	//escSeq_clearRow
	4,	//escSeq_styleReset
	3	//escSeq_scrollReset
};



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







//------ Functions ------

/**
 * @brief Encode ansi color into a single byte
 *
 */ 
Krle_ansiStyle krle_encodeStyle(const uint8_t bright, const uint8_t back, const uint8_t color, const uint8_t style){
	Krle_ansiStyle code = {
		.color 	=  color		& 0b111,
		.back		=  back		& 0b1111,
		.bright  = 	bright	& 0b1,
		.style 	=  style		& 0b111
	};
	return code;
}

/**
 * @brief Encode ansi color into a single byte
 *
 */ 
Krle_ansiStyle krle_decodeStyle(const uint8_t byte){
	Krle_ansiStyle code = {.byte=byte};
	return code;  
}