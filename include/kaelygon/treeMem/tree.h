/**
 * @file tree.h
 * @brief 16-bit uint c++ std::vector like data
 * Can hold any same width type in a single tree 
 */
#pragma once

#include <stdlib.h>
#include <stdint.h>


typedef struct{
	void *data;
	uint16_t length; //number of elements
	uint16_t width; //one element byte width
	uint16_t capacity; //available memory
	uint16_t maxLength; //maximum allowed number of elements before address overflow
}KaelTree;

uint8_t kaelTree_alloc(KaelTree *tree, uint16_t width);
void kaelTree_free(KaelTree *tree);

KaelTree *kaelTree_push(KaelTree *tree, const void *restrict element);
uint8_t kaelTree_pop(KaelTree *tree);
uint8_t kaelTree_resize(KaelTree *tree, uint16_t n);

void kaelTree_setWidth(KaelTree *tree, uint16_t width);

void kaelTree_set(KaelTree *tree, uint16_t index, const void *restrict element);

void *kaelTree_get(KaelTree *tree, uint16_t index);

void *kaelTree_begin(KaelTree *tree);
void *kaelTree_back(KaelTree *tree);

uint16_t kaelTree_length(KaelTree *tree);
uint16_t kaelTree_empty(KaelTree *tree);

