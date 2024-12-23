
// 
/**
	@file tree.c

	@brief 16-bit uint c++ std::vector like, dynamic memory stored in heap
	
	(void *)data can hold any type of an element, structs, pointers or even *KaelTree
	Memory is allocated dynamically, see growth/shrink factor macros below

	@code
	{ 
	//TODO: No mo nested recursive trees plz
	}
	@endcode
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/treeMem/tree.h"
#include "kaelygon/math/math.h"

//if used memory exceeds capacity, scale it by GROWTH_NUMER/GROWTH_DENOM times
//if capacity requires is less than GROWTH_DENOM/GROWTH_NUMER, it's scaled by that amount
#define GROWTH_NUMER 3 
#define GROWTH_DENOM 2 

#define ELEMS_MAX 32767 // (2^16-1)/GROWTH_FACTOR
#define ELEMS_MIN 0

//---alloc and free---

/**
 * @brief Initialize and allocate tree
 */
uint8_t kaelTree_alloc(KaelTree *tree, uint16_t size) {
	if(NULL_CHECK(tree)){return KAEL_ERR_NULL;}
	tree->length = 0;
	tree->data 		= NULL;
	tree->capacity 	= 0;
	kaelTree_setWidth(tree, size);
	return KAEL_SUCCESS;
}

/**
 * @brief Free including all the elements
 * 
 * @note Make sure to free allocated elements in tree 
*/
void kaelTree_free(KaelTree *tree){
	if(NULL_CHECK(tree,"free") || NULL_CHECK(tree->data,"free->data")){return;} 
	free(tree->data); //Free branch or leaf
	memset(tree,0,sizeof(KaelTree)); //set to NULL and 0
}


//---rescaling---

/**
 * @brief Add element to tree. NULL element is initialized as zero
 */
KaelTree *kaelTree_push(KaelTree *tree, const void *restrict element){
	if(NULL_CHECK(tree)){return NULL;}

	uint16_t newLength = tree->length +1;

	if(newLength > tree->maxLength){ //Beyond last growth stage
		printf("Too many elements\n");
    	return NULL; //return last element
	} 
	
	uint16_t newAlloc = (newLength+ELEMS_MIN) * tree->width;

	if( (newAlloc > tree->capacity) ){ //grow if above threshold
		if(newAlloc > (UINT16_MAX / GROWTH_NUMER) * GROWTH_DENOM){ //prevent overflow
			newAlloc = UINT16_MAX;
		}else{
			newAlloc = newAlloc * GROWTH_NUMER/GROWTH_DENOM;
		}

		void *newData = realloc(tree->data, newAlloc);
		if(NULL_CHECK(newData,"pushRealloc")){ return NULL; }
		tree->capacity=newAlloc;
		tree->data=newData;
	}
	tree->length=newLength;

	//copy the element after last element
    void *dest = kaelTree_back(tree); 
	if(NULL_CHECK(dest)){return NULL;}
	
	if(element==NULL){ //No macro since NULL use is valid
    	memset(dest, 0, tree->width);
	}else{
    	memcpy(dest, element, tree->width);
	}

	return dest;
}

/**
 * @brief Remove last element
 */
uint8_t kaelTree_pop(KaelTree *tree){
	if(NULL_CHECK(tree) || (tree->length==0)){return KAEL_ERR_NULL;}

	uint16_t newLength = tree->length -1;

	uint16_t newAlloc = newLength * tree->width + ELEMS_MIN;
	uint16_t scaleAlloc = (tree->capacity/GROWTH_NUMER)*GROWTH_DENOM;
	if( newAlloc <= scaleAlloc ){ //shrink if below threshold
		newAlloc = tree->capacity/GROWTH_NUMER*GROWTH_DENOM;
		void *newData = realloc(tree->data, newAlloc);
		if( NULL_CHECK(newData,"popRealloc") ){ return KAEL_ERR_ALLOC; }
		tree->capacity=newAlloc;
		tree->data=newData;
	}

	tree->length=newLength;
	return KAEL_SUCCESS;
}

/**
 * @brief Set tree to specific size
 */
uint8_t kaelTree_resize(KaelTree *tree, uint16_t length){
	if(NULL_CHECK(tree,"resize")){return KAEL_ERR_NULL;}

	uint16_t newLength = kaelMath_min(length, ELEMS_MAX);
	uint16_t minAlloc = newLength + ELEMS_MIN;
	uint16_t newAlloc = minAlloc * tree->width;

	//We rather keep ->data null till it has non zero allocation
	if(newAlloc!=0){ 
		void *newData = realloc(tree->data, newAlloc);
		if( NULL_CHECK(newData) ){ return KAEL_ERR_ALLOC; }

		//Zero newly resized portion if any
		if (newAlloc > tree->capacity) {
			uint16_t newSize = newAlloc - tree->capacity;
			memset((uint8_t*)newData + tree->capacity, 0, newSize);
		}

		tree->capacity=newAlloc;
		tree->data=newData;
	}

	tree->length=newLength;
	return KAEL_SUCCESS;
}

//---setters---

//set element byte width. Any existing data will be invalidated
void kaelTree_setWidth(KaelTree *tree, uint16_t size){
	if(NULL_CHECK(tree,"setSize")){return;}
	tree->width=size;
	uint16_t length = tree->length;
	tree->maxLength = UINT16_MAX / tree->width - ELEMS_MIN;
	kaelTree_resize(tree,length); //resize with new byte width
}

/**
 * @brief Set element value in a tree by index
 */
void kaelTree_set(KaelTree *tree, uint16_t index, const void *restrict element){
	if(NULL_CHECK(tree,"set")){return;}
	void *dest = kaelTree_get(tree, index);
	if(dest==NULL){return;}
	memcpy(dest, element, tree->width);
}

//---getters---

/**
 * @brief Return number of elements in tree 
 */
uint16_t kaelTree_length(KaelTree *tree){
	if(NULL_CHECK(tree)){return 0;}
	return tree->length;
}

uint16_t kaelTree_empty(KaelTree *tree){
	if(NULL_CHECK(tree)){return 1;}
	return (tree->length==0);
}

//---get pointers---

//get by index
void *kaelTree_get(KaelTree *tree, uint16_t index){
	if(NULL_CHECK(tree,"get")){return NULL;}
	if(NULL_CHECK(tree->data,"get")){return NULL;}
	#if KAEL_DEBUG==1
		if(index >= tree->length){
			KAEL_ERROR_NOTE("kaelTree_get out of bounds"); 
			return NULL;	
		}
	#endif
   void *elem = (uint8_t *)tree->data + index * tree->width;
	return elem;
}
//get first element
void *kaelTree_begin(KaelTree *tree){
    void *elem = kaelTree_get(tree,0);
	return elem;
}
//get last element
void *kaelTree_back(KaelTree *tree){
    void *elem = kaelTree_get( tree, tree->length-1 );
	return elem;
}