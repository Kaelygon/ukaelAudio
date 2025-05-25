/**
 * @file krleBase.h
 * 
 * @brief Header, KRLE standard minimal definitions 
 * 
 */
#pragma once

#include <stdlib.h>
#include <stdint.h>





//------ Global variables ------
//TODO: update style doc. all caps is bit of an eye sore so maybe we'll rethink on that stylization

//--- Constant values ---

extern const char *krle_constChar[];
extern const uint8_t krle_constCharLen[];

extern const uint8_t krle_defaultColorTable[32];
extern const uint8_t krle_defaultAttributeTable[8];

//--- Mutable global values ---

extern const uint8_t *krle_colorTable;
extern const uint8_t *krle_attributeTable;




//------ Typedef ------

/**
 * @brief Ansi color code escape sequence encoded in a single byte
 * Text mode ansiStyle byte
 * 
 * @warning At least one value has to be non-zero 
 */
typedef union {
	struct {
		uint8_t color  : 3; //3bit ansi SGR standard color 
		uint8_t back   : 1; //Background or Foreground: 0=FG, 1=BG
		uint8_t bright : 1; //Normal or Bright: 0=Normal, 1=Bright 
		uint8_t style  : 3; //SGR attribute
	};
	uint8_t byte;
}Krle_ansiStyle;





//------ KRLE header ------


typedef enum{
	KRLE_PIXEL_MODE_FLAG = 0b00000001,
	KRLE_TEXT_MODE_FLAG  = 0,
}Krle_headerFlags;

/**
 * @brief Krle_headerFlags can be OR:ed together
 */
typedef union {
	struct {
		uint8_t drawMode	: 1; //1 = Pixel mode, 0 = Text mode
		uint8_t unused 	: 7; 
	};
	uint8_t byte;
}Krle_flagByte;


typedef struct{
	//Image size in pixels
	uint16_t width;
	uint16_t height; //"Squashed height"

	//Ansi color codes
	//[8x Dim FG], [8x Dim BG], [8x Bright FG], [8x Bright BG]
	uint8_t palette[32]; 

	//Which SGR attributes .style index corresponds
	uint16_t attribute[8];		

	//Height stretch factor. Terminal characters are roughly 2 high 1 wide, ratio=2  
	uint8_t ratio; 

	//Data length in bytes after header
	uint32_t	length;		

	//1 bit flags
	Krle_flagByte flags;

}Krle_header;





//------ Enumerators ------

//--- Pixel mode ---

typedef enum{
	KRLE_SUCCESS				= 0,
	KRLE_ERR_ZERO_JUMP		= 128,
	KRLE_ERR_PIXEL_OVERFLOW = 129,
}krle_error;

/**
 * @brief Draw pixel mode special markers 
 *
 * @note
 * Pixel mode
 * Pixel runs [color : 4bit] [length : 4bit]
 * Call marker every jump 	[pixelJump : 8bit] [length : 8bit] ...
 * 
 * Zero length pixels are not valid, freeing values 0bABCD000 for markers
 * 
 * byte nibbles = [hi : 4bit, lo: 4bit]
*/
typedef enum{
	KRLE_PIXEL_JUMP		= 2<<4u, //Advance by next (uint8_t)byte in run mode
}Krle_drawModeMarker;

/**
 * @brief Text mode special markers
 * 
 * @note
 * Formatting in string
 * | [marker]		| [data byte]     |
 * | -----------  | --------------- |
 * | markerStyle	| [ansiCode.byte] | //Print ansi font style esc seq
 * | markerSpace	| [space count]   | //Print white space
 * | markerJump	| [jump count]   	| //Space to character without overdraw
 * 
 */
/**
 * @brief Special instructions stored as unicode PUA
 * 0xE0 to 0xF8
*/
typedef enum{
	KRLE_TEXT_STYLE = 0xE0,
	KRLE_TEXT_SPACE,
	KRLE_TEXT_JUMP,
	KRLE_TEXT_PLACEHOLDER = 0xF8
}krle_textModeMarker;





//------ Text mode ------

/**
 * @brief Enumerates *krle_constChar[] and krle_constCharLen[]
 */
typedef enum{
	KRLE_CLEAR,
	KRLE_CLEAR_ROW,
	KRLE_STYLE_RESET,
	KRLE_SCROLL_RESET
}krle_stringIndex;

//ANSI FG/Bright

typedef enum{
	ANSI_FG,
	ANSI_BG,
}krle_ansiColorTarget;

typedef enum{
	ANSI_NORMAL,
	ANSI_BRIGHT,
}krle_ansiColorIntensity;

/**
 * @brief font color
 * krle_colorTable indices
 * color format FG/BG and normal/bright stripped [color  : 3 bit]
 */
typedef enum {
	ANSI_BLACK 		= 0b000,
	ANSI_RED			, 
	ANSI_GREEN		, 
	ANSI_YELLOW		, 
	ANSI_BLUE		, 
	ANSI_MAGENTA	, 
	ANSI_CYAN		, 
	ANSI_WHITE		, 
}krle_colorTableIndex;

/**
 * @brief SGR ansi attributes
 * 
 * https://www.ecma-international.org/wp-content/uploads/ECMA-48_5th_edition_june_1991.pdf --- Page 61 (75/108)
 */
typedef enum {
	ANSI_RESET = 0xFF, //custom since 0 would be NULL, will overlap with text mode style Krle_ansiStyle(7,1,1,7)

	ANSI_NONE = 0, //0 would be same result as KRLE_STYLE_RESET
	ANSI_BOLD = 1,
	ANSI_FAINT = 2,
	ANSI_ITALIC = 3,
	ANSI_UNDERLINE = 4,
	ANSI_BLINK_SLOW = 5,
	ANSI_BLINK_FAST = 6,
	ANSI_REVERSE = 7,
	ANSI_CONCEALED = 8,
	ANSI_CROSSED_OUT = 9,

	ANSI_FONT_DEFAULT = 10,
	ANSI_FONT_1 = 11,
	ANSI_FONT_2 = 12,
	ANSI_FONT_3 = 13,
	ANSI_FONT_4 = 14,
	ANSI_FONT_5 = 15,
	ANSI_FONT_6 = 16,
	ANSI_FONT_7 = 17,
	ANSI_FONT_8 = 18,
	ANSI_FONT_9 = 19,

	ANSI_DOUBLE_UNDERLINE = 21,
	ANSI_NORMAL_INTENSITY = 22,
	ANSI_NO_ITALIC = 23,
	ANSI_NO_UNDERLINE = 24,
	ANSI_NO_BLINK = 25,
	ANSI_POSITIVE_IMAGE = 27,
	ANSI_REVEALED = 28,
	ANSI_NOT_CROSSED_OUT = 29,

	ANSI_FG_BLACK 		= 30,
	ANSI_FG_RED 		= 31,
	ANSI_FG_GREEN	 	= 32,
	ANSI_FG_YELLOW 	= 33,
	ANSI_FG_BLUE 		= 34,
	ANSI_FG_MAGENTA 	= 35,
	ANSI_FG_CYAN 		= 36,
	ANSI_FG_WHITE	 	= 37,
	ANSI_FG_DEFAULT	= 39,

	ANSI_BG_BLACK 		= 40,
	ANSI_BG_RED 		= 41,
	ANSI_BG_GREEN 		= 42,
	ANSI_BG_YELLOW 	= 43,
	ANSI_BG_BLUE 		= 44,
	ANSI_BG_MAGENTA 	= 45,
	ANSI_BG_CYAN 		= 46,
	ANSI_BG_WHITE 		= 47,
	ANSI_BG_DEFAULT 	= 49,

	ANSI_FG_BRIGHT_BLACK 	= 90,
	ANSI_FG_BRIGHT_RED 		= 91,
	ANSI_FG_BRIGHT_GREEN	 	= 92,
	ANSI_FG_BRIGHT_YELLOW 	= 93,
	ANSI_FG_BRIGHT_BLUE 		= 94,
	ANSI_FG_BRIGHT_MAGENTA 	= 95,
	ANSI_FG_BRIGHT_CYAN 		= 96,
	ANSI_FG_BRIGHT_WHITE	 	= 97,

	ANSI_BG_BRIGHT_BLACK 	= 100,
	ANSI_BG_BRIGHT_RED 		= 101,
	ANSI_BG_BRIGHT_GREEN 	= 102,
	ANSI_BG_BRIGHT_YELLOW 	= 103,
	ANSI_BG_BRIGHT_BLUE 		= 104,
	ANSI_BG_BRIGHT_MAGENTA 	= 105,
	ANSI_BG_BRIGHT_CYAN 		= 106,
	ANSI_BG_BRIGHT_WHITE 	= 107,

	ANSI_FRAMED = 51,
	ANSI_ENCIRCLED = 52,
	ANSI_OVERLINED = 53,
	ANSI_NOT_FRAMED = 54,
	ANSI_NOT_OVERLINED = 55
} krle_SGRAttributes;



//------ Functions ------

Krle_ansiStyle krle_encodeStyle(const uint8_t bright, const uint8_t back, const uint8_t color, const uint8_t style);
Krle_ansiStyle krle_decodeStyle(const uint8_t byte);