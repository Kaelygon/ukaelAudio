//./include/mem/treeMem.h
// 16-bit uint c++ std::vector like data
#pragma once

#include <stdlib.h>
#include <stdint.h>


typedef struct{
	void *data;
	uint16_t elemCount; //number of elements
	uint16_t width; //one element byte width
	uint16_t capacity; //available memory
	uint16_t maxElemCount; //maximum allowed elements
	uint8_t height; //Number of recursions to grandestchild
}KaelTree;

uint8_t kaelTree_alloc(KaelTree *mem, uint16_t width);
void kaelTree_free(KaelTree *mem);

KaelTree *kaelTree_push(KaelTree *mem, const void *element);
uint8_t kaelTree_pop(KaelTree *mem);
uint8_t kaelTree_resize(KaelTree *mem, uint16_t n);

void kaelTree_setWidth(KaelTree *mem, uint16_t width);
void kaelTree_setHeight(KaelTree *mem, uint16_t height);

void kaelTree_set(KaelTree *mem, uint16_t index, const void *element);

void *kaelTree_get(KaelTree *mem, uint16_t index);

void *kaelTree_begin(KaelTree *mem);
void *kaelTree_back(KaelTree *mem);

uint16_t kaelTree_count(KaelTree *mem);
uint16_t kaelTree_empty(KaelTree *mem);

