
// 
/**
	@file tree.c

	@brief 16-bit uint c++ std::vector like, dynamic memory stored in heap
	
	@note No bounds checking
	
	(void *)data can hold any type of an element, structs, pointers or even *KaelTree itself
	Memory is allocated dynamically, see growth/shrink factor macros below

	@code
	{ 
	//TODO:
	}
	@endcode
	
*/

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/treeMem/tree.h"
#include "kaelygon/math/math.h"

//if used memory exceeds capacity, scale it by GROWTH_NUMER/GROWTH_DENOM times
//if capacity requires is less than GROWTH_DENOM/GROWTH_NUMER, it's scaled by that amount
#define GROWTH_NUMER 3 
#define GROWTH_DENOM 2 

#define ELEMS_MAX (UINT16_MAX-1) // -1 that push ->length+1 can be done without a range change

//---alloc and free---

/**
 * @brief Initialize and allocate tree
 * @return KAEL_SUCCESS or KAEL_ERR_NULL
 */
uint8_t kaelTree_alloc(KaelTree *tree, const uint16_t size){
	if(NULL_CHECK(tree)){return KAEL_ERR_NULL;}
	tree->length = 0;
	tree->data = NULL;
	tree->capacity	= 0;
	tree->reserve = 0;
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
 * @brief Set apparent tree length. Shrinking won't invoke realloc
 * @return Kael_infoCode
 */
uint8_t kaelTree_resize(KaelTree *tree, const uint16_t length){
	if(NULL_CHECK(tree,"resize")){return KAEL_ERR_NULL;}

	if(length > tree->maxLength){ //Beyond last growth stage
		printf("Too many elements\n");
    	return KAEL_ERR_FULL;
	} 

	uint16_t newLength = kaelMath_min(length, ELEMS_MAX);
	uint16_t minAlloc = kaelMath_max(newLength, tree->reserve);
	uint16_t newAlloc = minAlloc * tree->width;

	if( newAlloc > tree->capacity ){ 
		if(newAlloc > (ELEMS_MAX / GROWTH_NUMER) * GROWTH_DENOM){ //prevent overflow
			newAlloc = ELEMS_MAX;
		}else{
			newAlloc = newAlloc * GROWTH_NUMER/GROWTH_DENOM;
		}

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

/**
 * @brief Reserve minimum number of elements
 * @return Kael_infoCode
 */
uint8_t kaelTree_reserve(KaelTree *tree, const uint16_t length){
	if(NULL_CHECK(tree,"reserve")){return KAEL_ERR_NULL;}
	tree->reserve = length;

	//Similar to resize but without growth scaling or setting length
	uint16_t newLength = kaelMath_min(length, ELEMS_MAX);
	uint16_t newAlloc = newLength * tree->width;

	if( newAlloc > tree->capacity ){ 
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

	return KAEL_SUCCESS;
}

/**
 * @brief Insert at index. NULL element is initialized as zero
 * @return On success return newly pushed element address. On fail return NULL 
 */
void *kaelTree_insert(KaelTree *tree, uint16_t index, const void *restrict element){
	if(NULL_CHECK(tree)){return NULL;}
	KAEL_ASSERT(index < tree->length, "kaelTree_insert out of bounds");
	uint8_t code = kaelTree_resize(tree, tree->length+1);

	//from index to old end
	uint16_t copyAmount = (tree->length - index) * tree->width;

	if(code==KAEL_ERR_FULL){return NULL;}

	//shift everything from index to end by 1 element
	void* dest = kaelTree_get(tree, index);
	if(dest==NULL){return NULL;}
	memmove( (uint8_t *)dest + tree->width, dest, copyAmount );

	if(element==NULL){
    	memset(dest, 0, tree->width);
	}else{
    	memcpy(dest, element, tree->width);
	}
	return dest;
}

/**
 * @brief Add element to tree. NULL element is initialized as zero
 * @return On success return newly pushed element address. On fail return NULL 
 */
void *kaelTree_push(KaelTree *tree, const void *restrict element){
	if(NULL_CHECK(tree)){return NULL;}
	
	uint8_t code = kaelTree_resize(tree, tree->length+1);
	if(code==KAEL_ERR_FULL){return NULL;}

	//copy the element after last element
   void *dest = kaelTree_back(tree); 
	if(NULL_CHECK(dest)){return NULL;}
	
	if(element==NULL){ 
    	memset(dest, 0, tree->width);
	}else{
    	memcpy(dest, element, tree->width);
	}

	return dest;
}

/**
 * @brief Remove last element
 * @return Kael_infoCode
 */
uint8_t kaelTree_pop(KaelTree *tree){
	if(NULL_CHECK(tree) || (tree->length==0)){return KAEL_ERR_NULL;}

	uint16_t newLength = tree->length -1;

	uint16_t minAlloc = kaelMath_max(newLength, tree->reserve);
	uint16_t newAlloc = tree->width * minAlloc;
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

//---setters---

/**
 * @brief Set element byte width. Any existing data will be invalidated
 */
void kaelTree_setWidth(KaelTree *tree, const uint16_t size){
	if(NULL_CHECK(tree,"setSize")){return;}
	tree->width=size;
	tree->maxLength = ELEMS_MAX / tree->width - tree->reserve;
	kaelTree_resize(tree, tree->length); //resize with new byte width
}

/**
 * @brief Set element value in a tree by index
 */
void kaelTree_set(KaelTree *tree, const uint16_t index, const void *restrict element){
	if(NULL_CHECK(tree,"set")){return;}
	void *dest = kaelTree_get(tree, index);
	if(dest==NULL){return;}
	memcpy(dest, element, tree->width);
}

//---getters---

/**
 * @brief Return number of elements in tree 
 */
uint16_t kaelTree_length(const KaelTree *tree){
	if(NULL_CHECK(tree)){return 0;}
	return tree->length;
}

//@return 1 if NULL or empty. Return 0 if not empty
uint16_t kaelTree_empty(const KaelTree *tree){
	if(NULL_CHECK(tree)){return 1;}
	return (tree->length==0);
}

//---get pointers---

//get by index
void *kaelTree_get(const KaelTree *tree, const uint16_t index){
	KAEL_ASSERT(!NULL_CHECK(tree,"get"));
	KAEL_ASSERT(!NULL_CHECK(tree->data,"get"));
	KAEL_ASSERT(index < tree->length, "kaelTree_get out of bounds");

   void *elem = (uint8_t *)tree->data + index * tree->width;
	return elem;
}
//get first element
void *kaelTree_begin(const KaelTree *tree){
   void *elem = kaelTree_get(tree,0);
	return elem;
}
//get last element
void *kaelTree_back(const KaelTree *tree){
   void *elem = kaelTree_get( tree, tree->length-1 );
	return elem;
}

//Set **current to next element 
void kaelTree_next(const KaelTree *tree, void **current){
	KAEL_ASSERT(current != NULL && tree != NULL);
	void *next = (uint8_t *)(*current) + tree->width;
	void *back = kaelTree_back(tree);
	if((void *)next > (void *)back){
		*current=NULL;
		return;
	}
	*current=next;
}

//Set **current to previous element 
void kaelTree_prev(const KaelTree *tree, void **current){
	KAEL_ASSERT(current != NULL && tree != NULL);
	void *prev = (uint8_t *)(*current) - tree->width;
	void *front = kaelTree_begin(tree);
	if((void *)prev < (void *)front){
		*current=NULL;
		return;
	}
	*current=prev;
}
