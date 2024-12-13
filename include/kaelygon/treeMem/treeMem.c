
//./include/mem/kaelData.c
// 16-bit uint c++ std::vector like, nested dynamic memory stored in heap
/*

	(void *)data can hold any type of an element, structs, pointers or even *KaelMem
	Memory is allocated dynamically, see growth/shrink factor macros below

	If (void *)data elements are *KaelMem, you MUST manually set the level of recursion by kaelMem_setHeight() 
	Height 0 means the elements are stored in (void *)data 

	You have to manage your own allocations stored in (void *)data before using kaelMem_free()
	kaelMem_free() will only clean *KaelMem recursion.


	{ //Example creating 1 height tree: (KaelMem *)tree -> (KaelMem *)branch -> (uint16_t)leaf
		KaelMem *tree = kaelMem_alloc(sizeof(KaelMem));
		kaelMem_setHeight(tree,1); //set number of levels (before reaching the leaves) to 1

		KaelMem *branch = kaelMem_push(tree,NULL); //add branch and get its pointer in return
		kaelMem_setSize(branch,sizeof(uint16_t)); //type size (byte width) must be set if NULL was pushed

		uint16_t leaf=123;
		kaelMem_push(branch,&leaf); //push leaf
		
		//if leaf had allocated memory, it would be freed here before del

		kaelMem_free(&tree); //this safely deletes all the branches and leaves if sizes and height were set correctly
		return 0;
	}
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/treeMem/treeMem.h"
#include "kaelygon/math/math.h"
#include "kaelygon/kaelMacros.h"

//if used memory exceeds capacity, scale it by GROWTH_NUMER/GROWTH_DENOM times
//if capacity requires is less than GROWTH_DENOM/GROWTH_NUMER, it's scaled by that amount
#define GROWTH_NUMER 3 
#define GROWTH_DENOM 2 

#define ELEMS_MAX 32767 // (2^16-1)/GROWTH_FACTOR
#define ELEMS_MIN 4

//---create and destroy---

//Create and allocate mem which elements are size bytes
KaelMem *kaelMem_alloc(uint16_t size) {
	KaelMem *newMem = malloc(sizeof(KaelMem)); 
	if (NULL_CHECK(newMem)) { return NULL; }

	newMem->elemCount 	= 0;
	newMem->data 		= NULL;
	newMem->capacity 	= ELEMS_MIN * size;
	kaelMem_setSize(newMem, size);
	newMem->height 		= 0;


	if (NULL_CHECK(newMem->data)) { free(newMem); return NULL; }
	
	return newMem;
}


// Free the tree in postorder traversal order
// Take first branch in each node until height 0 is reached
// Free all the leaves and go up 1 height and do the same for the next branch
// Repeat until height root is reached. Root is not freed.
uint8_t kaelMem_freeRecursive(KaelMem *mem){
	if(NULL_CHECK(mem)){return 0;}
	static uint16_t depth = 0; //keep track of recursion
	depth++;
	if( mem->height!=0 ){ //free all the branches stored as (KaelMem *) recursively
		for(uint16_t i=0; i< mem->elemCount; i++ ){ //free every branch
			KaelMem *descendant = (KaelMem *)kaelMem_get(mem,i);
			kaelMem_freeRecursive(descendant); //Lowest nodes has to be freed first or the entire branch is lost
		}
	} //else mem->data is leaves
	free(mem->data);
	mem->data=NULL;
	depth--;
	return depth;
}

//Free and Delete the whole structure
void kaelMem_free(KaelMem **mem){
	if(NULL_CHECK(*mem)){return;}
	//free trunk (if any) and leaves
	uint16_t depth = kaelMem_freeRecursive(*mem); 
	if(depth==0){ //free root (*self)
		free(*mem);
		*mem=NULL;
	}
}


//---rescaling---
//Pushing NULL pushes zeroed element
KaelMem *kaelMem_push(KaelMem *mem, const void *element){
	if(NULL_CHECK(mem)){return NULL;}

	uint16_t newCount = mem->elemCount +1;

	if(newCount > mem->maxElemCount){ //Beyond last growth stage
		printf("Too many elements\n");
    	return NULL; //return last element
	} 
	
	uint16_t newAlloc = (newCount+ELEMS_MIN) * mem->size;

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
    void *dest = kaelMem_back(mem); 
	if(NULL_CHECK(dest)){return NULL;}
	
	if(element==NULL){ //No macro since NULL use is valid
    	memset(dest, 0, mem->size);
	}else{
    	memcpy(dest, element, mem->size);
	}

	return dest;
}

//Remove last element
uint8_t kaelMem_pop(KaelMem *mem){
	if(NULL_CHECK(mem) || (mem->elemCount==0)){return KAEL_ERR_NULL;}
	
	//If height>0, we are popping a branch which descendants have to be freed
	if(mem->height > 0){
		kaelMem_freeRecursive(kaelMem_back(mem));
	}

	uint16_t newCount = mem->elemCount -1;

	uint16_t newAlloc = newCount * mem->size + ELEMS_MIN;
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

uint8_t kaelMem_resize(KaelMem *mem, uint16_t elemCount){
	if(NULL_CHECK(mem,"resize")){return KAEL_ERR_NULL;}

	uint16_t newCount = kaelMath_min(elemCount, ELEMS_MAX);
	uint16_t minAlloc = newCount+ELEMS_MIN;
	uint16_t newAlloc = minAlloc * mem->size;

	void *newData = realloc(mem->data, newAlloc);
	if( NULL_CHECK(newData) ){ return KAEL_ERR_MEM; }
	mem->capacity=newAlloc;
	mem->data=newData;

	mem->elemCount=newCount;
	return KAEL_SUCCESS;
}

//---setters---

//set element byte width. Any existing data will be invalidated
void kaelMem_setSize(KaelMem *mem, uint16_t size){
	if(NULL_CHECK(mem,"setSize")){return;}
	mem->size=size;
	uint16_t elemCount = mem->elemCount;
	mem->maxElemCount = UINT16_MAX / mem->size - ELEMS_MIN;
	kaelMem_resize(mem,elemCount); //resize with new byte width
}

void kaelMem_set(KaelMem *mem, uint16_t index, const void *element){
	if(NULL_CHECK(mem,"set")){return;}
	void *dest = kaelMem_get(mem, index);
	memcpy(dest, element, mem->size);
}

void kaelMem_setHeight(KaelMem *mem, uint16_t height){
	if(NULL_CHECK(mem)){return;}
	mem->height=height;
}

//---getters---

uint16_t kaelMem_count(KaelMem *mem){
	if(NULL_CHECK(mem)){return 0;}
	return mem->elemCount;
}

uint16_t kaelMem_empty(KaelMem *mem){
	if(NULL_CHECK(mem)){return 1;}
	return (mem->elemCount==0);
}

//---get pointers---

//get by index
void *kaelMem_get(KaelMem *mem, uint16_t index){
	if(NULL_CHECK(mem,"get")){return NULL;}
	if(NULL_CHECK(mem->data,"get")){return NULL;}
	#if KAEL_DEBUG==1
		if(index >= mem->elemCount){
			printf("Index out of bounds\n"); 
			return NULL;	
		}
	#endif
    void *elem = (uint8_t *)mem->data + index * mem->size;
	return elem;
}
//get first element
void *kaelMem_begin(KaelMem *mem){
    void *elem = kaelMem_get(mem,0);
	return elem;
}
//get last element
void *kaelMem_back(KaelMem *mem){
    void *elem = kaelMem_get( mem, mem->elemCount-1 );
	return elem;
}
