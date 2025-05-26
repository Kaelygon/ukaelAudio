/**
 * @file krleText.h
 * 
 * @brief Header, krle text mode tools
 * 
*/
#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "krle/krleBase.h"

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

//------ Functions ------

Krle_ansiStyle krle_encodeStyle(const uint8_t bright, const uint8_t back, const uint8_t color, const uint8_t style);
Krle_ansiStyle krle_decodeStyle(const uint8_t byte);