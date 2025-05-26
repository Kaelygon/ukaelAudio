/**
 * @file krleText.c
 * 
 * @brief Implementation, krle text mode tools
 * 
*/

#include "krle/krleText.h"

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