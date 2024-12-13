
// 
/**
	@file treeMem.c

	@brief 16-bit uint c++ std::vector like, nested dynamic memory stored in heap
	
	(void *)data can hold any type of an element, structs, pointers or even *KaelTree
	Memory is allocated dynamically, see growth/shrink factor macros below

	If (void *)data elements are *KaelTree, you MUST manually set the level of recursion by kaelTree_setHeight() 
	Height 0 means the elements are stored in (void *)data 

	You have to manage your own allocations stored in (void *)data before using kaelTree_free()
	kaelTree_free() will only clean *KaelTree recursion.

	@code
	{ //Example creating 1 height tree: (KaelTree *)tree -> (KaelTree *)branch -> (uint16_t)leaf
		KaelTree *tree = kaelTree_alloc(sizeof(KaelTree));
		kaelTree_setHeight(tree,1); //set number of levels (before reaching the leaves) to 1

		KaelTree *branch = kaelTree_push(tree,NULL); //add branch and get its pointer in return
		kaelTree_setWidth(branch,sizeof(uint16_t)); //type size (byte width) must be set if NULL was pushed

		uint16_t leaf=123;
		kaelTree_push(branch,&leaf); //push leaf
		
		//if leaf had allocated memory, it would be freed here before del

		kaelTree_free(&tree); //this safely deletes all the branches and leaves if sizes and height were set correctly
		return 0;
	}
	@endcode
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/treeMem/treeMem.h"
#include "kaelygon/math/math.h"
#include "kaelygon/global/kaelMacros.h"

//if used memory exceeds capacity, scale it by GROWTH_NUMER/GROWTH_DENOM times
//if capacity requires is less than GROWTH_DENOM/GROWTH_NUMER, it's scaled by that amount
#define GROWTH_NUMER 3 
#define GROWTH_DENOM 2 

#define ELEMS_MAX 32767 // (2^16-1)/GROWTH_FACTOR
#define ELEMS_MIN 4

//---alloc and free---

/**
 * @brief Initialize and allocate tree
 */
uint8_t kaelTree_alloc(KaelTree *mem, uint16_t size) {
	if(NULL_CHECK(mem)){return KAEL_ERR_NULL;}

	mem->elemCount = 0;
	mem->data 		= NULL;
	mem->capacity 	= ELEMS_MIN * size;
	kaelTree_setWidth(mem, size);
	mem->height 	= 0;

	if (NULL_CHECK(mem->data)) { return KAEL_ERR_MEM; }
	
	return 0;
}

/**
 * @brief Free the tree in post traversal order
 * 
 * @details Take first branch in each node until height 0 is reached
 * Free all the leaves and go up 1 height and do the same for the next branch
 * Repeat until height root is reached. Root is not freed.
*/
//Free the tree in post traversal order
void kaelTree_free(KaelTree *mem){
	if(NULL_CHECK(mem) || NULL_CHECK(mem->data)){return;} 
	//capacity should never be 0 for non-null ptr. If it is, you set height wrong or have uninitialized ptr
	if(mem->capacity==0){ KAEL_ERROR_NOTE("kaelTree_free 0 capacity,"); return;} 

	if( mem->height!=0 ){ //height > 0 is a branch; height == 0 is a leaf
		for(uint16_t i=0; i< mem->elemCount; i++ ){ //Iterate every node in the branch
			KaelTree *descendant = (KaelTree *)kaelTree_get(mem,i);
			kaelTree_free(descendant); //Lowest nodes has to be freed first or the entire branch is lost
		}
	}
	free(mem->data); //Free branch or leaf
	memset(mem,0,sizeof(KaelTree)); //set to NULL and 0
}


//---rescaling---

/**
 * @brief Add element to tree. NULL element is initialized as zero
 */
KaelTree *kaelTree_push(KaelTree *mem, const void *element){
	if(NULL_CHECK(mem)){return NULL;}

	uint16_t newCount = mem->elemCount +1;

	if(newCount > mem->maxElemCount){ //Beyond last growth stage
		printf("Too many elements\n");
    	return NULL; //return last element
	} 
	
	uint16_t newAlloc = (newCount+ELEMS_MIN) * mem->width;

	if( (newAlloc > mem->capacity) ){ //grow if above threshold
		if(newAlloc > (UINT16_MAX / GROWTH_NUMER) * GROWTH_DENOM){ //prevent overflow
			newAlloc = UINT16_MAX;
		}else{
			newAlloc = newAlloc * GROWTH_NUMER/GROWTH_DENOM;
		}

		void *newData = realloc(mem->data, newAlloc);
		if(NULL_CHECK(newData,"pushRealloc")){ return NULL; }
		mem->capacity=newAlloc;
		mem->data=newData;
	}
	mem->elemCount=newCount;

	//copy the element after last element
    void *dest = kaelTree_back(mem); 
	if(NULL_CHECK(dest)){return NULL;}
	
	if(element==NULL){ //No macro since NULL use is valid
    	memset(dest, 0, mem->width);
	}else{
    	memcpy(dest, element, mem->width);
	}

	return dest;
}

/**
 * @brief Remove last element
 */
uint8_t kaelTree_pop(KaelTree *mem){
	if(NULL_CHECK(mem) || (mem->elemCount==0)){return KAEL_ERR_NULL;}
	
	//If height>0, we are popping a branch which descendants have to be freed
	if(mem->height > 0){
		kaelTree_free(kaelTree_back(mem));
	}

	uint16_t newCount = mem->elemCount -1;

	uint16_t newAlloc = newCount * mem->width + ELEMS_MIN;
	uint16_t scaleAlloc = (mem->capacity/GROWTH_NUMER)*GROWTH_DENOM;
	if( newAlloc <= scaleAlloc ){ //shrink if below threshold
		newAlloc = mem->capacity/GROWTH_NUMER*GROWTH_DENOM;
		void *newData = realloc(mem->data, newAlloc);
		if( NULL_CHECK(newData,"popRealloc") ){ return KAEL_ERR_MEM; }
		mem->capacity=newAlloc;
		mem->data=newData;
	}

	mem->elemCount=newCount;
	return KAEL_SUCCESS;
}

/**
 * @brief Set tree to specific size
 */
uint8_t kaelTree_resize(KaelTree *mem, uint16_t elemCount){
	if(NULL_CHECK(mem,"resize")){return KAEL_ERR_NULL;}

	uint16_t newCount = kaelMath_min(elemCount, ELEMS_MAX);
	uint16_t minAlloc = newCount + ELEMS_MIN;
	uint16_t newAlloc = minAlloc * mem->width;

	//We rather keep ->data null till it has non zero allocation
	if(newAlloc!=0){ 
		void *newData = realloc(mem->data, newAlloc);
		if( NULL_CHECK(newData) ){ return KAEL_ERR_MEM; }

        //Zero newly resized portion if any
        if (newAlloc > mem->capacity) {
            uint16_t newSize = newAlloc - mem->capacity;
            memset((uint8_t*)newData + mem->capacity, 0, newSize);
        }

		mem->capacity=newAlloc;
		mem->data=newData;
	}

	mem->elemCount=newCount;
	return KAEL_SUCCESS;
}

//---setters---

//set element byte width. Any existing data will be invalidated
void kaelTree_setWidth(KaelTree *mem, uint16_t size){
	if(NULL_CHECK(mem,"setSize")){return;}
	mem->width=size;
	uint16_t elemCount = mem->elemCount;
	mem->maxElemCount = UINT16_MAX / mem->width - ELEMS_MIN;
	kaelTree_resize(mem,elemCount); //resize with new byte width
}

/**
 * @brief set value by index
 */
void kaelTree_set(KaelTree *mem, uint16_t index, const void *element){
	if(NULL_CHECK(mem,"set")){return;}
	void *dest = kaelTree_get(mem, index);
	if(dest==NULL){return;}
	memcpy(dest, element, mem->width);
}

void kaelTree_setHeight(KaelTree *mem, uint16_t height){
	if(NULL_CHECK(mem)){return;}
	mem->height=height;
}

//---getters---

uint16_t kaelTree_count(KaelTree *mem){
	if(NULL_CHECK(mem)){return 0;}
	return mem->elemCount;
}

uint16_t kaelTree_empty(KaelTree *mem){
	if(NULL_CHECK(mem)){return 1;}
	return (mem->elemCount==0);
}

//---get pointers---

//get by index
void *kaelTree_get(KaelTree *mem, uint16_t index){
	if(NULL_CHECK(mem,"get")){return NULL;}
	if(NULL_CHECK(mem->data,"get")){return NULL;}
	#if KAEL_DEBUG==1
		if(index >= mem->elemCount){
			KAEL_ERROR_NOTE("kaelTree_get out of bounds"); 
			return NULL;	
		}
	#endif
   void *elem = (uint8_t *)mem->data + index * mem->width;
	return elem;
}
//get first element
void *kaelTree_begin(KaelTree *mem){
    void *elem = kaelTree_get(mem,0);
	return elem;
}
//get last element
void *kaelTree_back(KaelTree *mem){
    void *elem = kaelTree_get( mem, mem->elemCount-1 );
	return elem;
}
