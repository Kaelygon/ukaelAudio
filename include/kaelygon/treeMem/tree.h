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

uint8_t kaelTree_alloc(KaelTree *tree, const uint16_t width);
void kaelTree_free(KaelTree *tree);

uint8_t kaelTree_resize(KaelTree *tree, const uint16_t n);
KaelTree *kaelTree_push(KaelTree *tree, const void *restrict element);
uint8_t kaelTree_pop(KaelTree *tree);
KaelTree *kaelTree_insert(KaelTree *tree, uint16_t index, const void *restrict element);

void kaelTree_setWidth(KaelTree *tree, const uint16_t width);

void kaelTree_set(KaelTree *tree, const uint16_t index, const void *restrict element);

void *kaelTree_get(const KaelTree *tree, uint16_t index);

void *kaelTree_begin(const KaelTree *tree);
void *kaelTree_back(const KaelTree *tree);

uint16_t kaelTree_length(const KaelTree *tree);
uint16_t kaelTree_empty(const KaelTree *tree);

void kaelTree_next(const KaelTree *tree, void **current);