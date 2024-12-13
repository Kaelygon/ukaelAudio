//./include/mem/kaelMem.h
// 16-bit uint c++ std::vector like data
#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef struct KaelMem KaelMem;

struct KaelMem{
	void *data;
	uint16_t elemCount; //number of elements
	uint16_t size; //one element size in bytes
	uint16_t capacity; //available memory, always non-zero
	uint16_t maxElemCount; //maximum allowed elements
	uint8_t height; //Number of recursions to grandestchild
};

KaelMem *kaelMem_push(KaelMem *mem, const void *element);
uint8_t kaelMem_pop(KaelMem *mem);
uint8_t kaelMem_resize(KaelMem *mem, uint16_t n);

KaelMem *kaelMem_alloc(uint16_t size);
void kaelMem_free(KaelMem **mem);

void kaelMem_set(KaelMem *mem, uint16_t index, const void *element);
void *kaelMem_get(KaelMem *mem, uint16_t index);

void *kaelMem_begin(KaelMem *mem);
void *kaelMem_back(KaelMem *mem);

void kaelMem_setSize(KaelMem *mem, uint16_t size);
void kaelMem_setHeight(KaelMem *mem, uint16_t height);

uint16_t kaelMem_count(KaelMem *mem);
uint16_t kaelMem_empty(KaelMem *mem);

