/**
 * @file k32.h
 * 
 * @brief kael32_t k32 by unsigned char functions
 * 
 * Most significant byte is first in array
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include "kaelygon/global/kaelMacros.h"


//--- k32 by u8

kael32_t k32_u8set(const uint8_t b);

uint8_t k32_u8mul(kael32_t *result, const kael32_t *base, const uint8_t mul);
uint8_t k32_u8mad(kael32_t *result, const kael32_t *base, const uint8_t mul, const uint8_t add);

uint8_t k32_u8div(kael32_t *result, const kael32_t *base, const uint8_t div);

uint8_t k32_u8add(kael32_t *result, const kael32_t *base, const uint8_t add);
uint8_t k32_u8sub(kael32_t *result, const kael32_t *base, const uint8_t sub);