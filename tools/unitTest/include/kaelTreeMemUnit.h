/**
 * @file kaelTreeMemUnit.h
 * 
 * @brief kaelTree unit test
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/treeMem/tree.h"
#include "kaelygon/math/math.h"

typedef struct{
	char* name;
	uint16_t length;
	uint16_t readHead;
	uint16_t start[2]; //Start row and column
	uint16_t end[2]; //End row and column
}unitTest_leaf;

void unitTest_treeAlloc(KaelTree *tree, uint16_t branchCount, uint16_t leafCount, uint16_t leafMaxLen){
	kaelTree_alloc(tree, sizeof(KaelTree)); //Tree holds branches 
	kaelTree_resize(tree, branchCount); //Resize allocates one chunk at once

	for(uint16_t i=0; i<branchCount; i++){
		KaelTree tmpBranch; //Ownership will be passed to tree
		kaelTree_alloc(&tmpBranch, sizeof(unitTest_leaf));
		kaelTree_resize(&tmpBranch, leafCount); //Allocate leaves to a branch

		for(uint16_t j=0; j<kaelTree_length(&tmpBranch); j++){ //Always safer to get length from tree incase resize was clamped
			unitTest_leaf tmpLeaf;
			tmpLeaf.name = malloc(leafMaxLen*sizeof(char));
			if(tmpLeaf.name==NULL){
				printf("Leaf: %u %u failed\n", i, j); 
				continue;
			}
			memset(tmpLeaf.name,'\0',leafMaxLen);

			tmpLeaf.length = leafMaxLen;
			tmpLeaf.readHead = 0;
			
			kaelTree_set(&tmpBranch, j, &tmpLeaf); //transfer leaf ownership of the memory to branch
		}
		kaelTree_set(tree, i, (void *)(&tmpBranch)); //transfer branch ownership to tree
	}//As we exit the scope, _get function is used to reobtain the branches and leaves memory from tree
}

void unitTest_treeFree(KaelTree *tree){
	uint16_t branchCount = kaelTree_length(tree); //get element count
	for(uint16_t i=0; i<branchCount; i++){
		KaelTree *branch = kaelTree_get(tree,i); //get element by index

		uint16_t leafCount = kaelTree_length(branch);
		for(uint16_t j=0; j<leafCount; j++){
			unitTest_leaf *leaf = kaelTree_get(branch,j);
			if(leaf==NULL){
				printf("Failed to free leaf %u %u\n", leaf->start[0], leaf->start[1]);
				continue;
			}
			//printf("%s\n",leaf->name);

			free(leaf->name);
			leaf->name=NULL;
		}
		kaelTree_free(branch);
	}
	kaelTree_free(tree);
}



/**
 * @brief Fill leaf with some rectangle
 */
void unitTest_leafGenerate(unitTest_leaf *leaf, uint16_t colCount, uint16_t rowCount, uint16_t leafMaxLen, uint16_t seed){

	seed = kaelRand_lcg(seed);
	leaf->start[0] = ((seed)%rowCount);
	leaf->start[1] = ((seed>>8)%colCount);

	seed = kaelRand_lcg(seed);
	leaf->end[1] = leaf->start[1] + kaelMath_sub(colCount,seed%colCount);
	uint16_t leafWidth = leaf->end[1] - leaf->start[1] + 1;
	leaf->end[0] = leaf->start[0]+(leafMaxLen+leafWidth-1)/leafWidth; //rows, height ceil
	uint16_t newLength = leaf->end[1]*leaf->end[0];
	newLength = kaelMath_min(newLength,leafMaxLen);

	const uint8_t ASCII_START = 32;
	const uint8_t ASCII_END = 126;
	for(uint16_t k=0; k<newLength; k++){
		leaf->name[k]	= seed%(ASCII_END-ASCII_START)+ASCII_START;
	}
}

/**
 * @brief Populate all branches
 */
void unitTest_treeGenerate(KaelTree *tree, uint16_t colCount, uint16_t rowCount, uint16_t leafMaxLen){
	uint16_t randNum = kaelRand_lcg(2141);
	uint16_t branchCount = kaelTree_length(tree);
	for(uint16_t i=0; i<branchCount; i++){
		KaelTree *branch = kaelTree_get(tree,i);

		uint16_t leafCount = kaelTree_length(branch);
		for(uint16_t j=0; j<leafCount; j++){
			unitTest_leaf *leaf = kaelTree_get(branch,j);
			if(leaf==NULL){
				printf("Failed to get leaf %u %u\n", leaf->start[0], leaf->start[1]);
				continue;
			}
			randNum = kaelRand_lcg(randNum);
			unitTest_leafGenerate(leaf, colCount, rowCount, leafMaxLen, randNum);
		}
	}
}

/**
 * @brief Print branch like it was a page
 */
void printPage(KaelTree *tree, uint16_t colCount, uint16_t rowCount, uint16_t branchID){

	KaelTree *branch = kaelTree_get(tree, branchID); 
	if(branch==NULL){ printf("Failed to read branch %u printPage\n", branchID); }

	uint16_t rowBufSize=colCount+1;
	char rowBuf[rowBufSize];

	//print like they were a page
	uint16_t leafCount = kaelTree_length(branch);
	for(uint16_t row=0; row<rowCount; row++){
		//reset buffer
		memset(rowBuf,' ',rowBufSize);

		//find leaves on current row
		for(uint16_t leafID=0; leafID<leafCount; leafID++){
			unitTest_leaf *leaf = kaelTree_get(branch,leafID);
			if (leaf->start[0] <= row && leaf->end[0] >= row) {
				if(leaf->start[1]>=rowBufSize){continue;} //column start out of bounds
				if(leaf->readHead >= leaf->length){continue;} //nothing left to print

				uint16_t lineWidth = leaf->end[1]-leaf->start[1];
				uint16_t copyAmount = leaf->length - leaf->readHead; //skip previously written rows

				uint16_t lengthToEnd = rowBufSize - leaf->start[1];
				copyAmount = kaelMath_min(copyAmount, lengthToEnd); //prevent overflow
				copyAmount = kaelMath_min(copyAmount, lineWidth); //copy max row width


				char *rowBufStart = rowBuf + leaf->start[1];
				char *leafNameStart = leaf->name + leaf->readHead;
				memcpy(rowBufStart, leafNameStart, copyAmount); //copy shape row to buffer

				leaf->readHead += lineWidth; //advance leaf readHead for next rows
			}
		}
		rowBuf[rowBufSize-1] = '\0'; //null terminate
		printf("%s\n",rowBuf); //print row

		//reset read heads
		for(uint16_t leafID=0; leafID<leafCount; leafID++){
			unitTest_leaf *leaf = kaelTree_get(branch,leafID);
			leaf->readHead=0;
		}
	}
		
}

/**
 * @brief Test branched structure, tree containing a tree
 */
void kaelTree_drawSquares_unit() {
	uint16_t branchCount = 1;
	uint16_t leafCount = 8; 
	uint16_t leafMaxLen = 64;
	uint16_t cols = 64;
	uint16_t rows = 16;

	KaelTree tree;

	unitTest_treeAlloc(&tree,branchCount,leafCount,leafMaxLen);

	unitTest_treeGenerate(&tree, cols, rows, leafMaxLen);
	//print second branch
	uint16_t branchID = 0;
	printPage(&tree, cols, rows, branchID);

	//Free memory
	unitTest_treeFree(&tree);


	printf("unitTest_treeGenerate Done\n");	
}








/**
 * @brief Test individual functions
 */
void kaelTree_functions_unit(){

	KaelTree tree;
	uint8_t code=KAEL_SUCCESS;
	uint16_t elementCount=13;
	uint16_t reserveCount=9;
	uint16_t setElementIndex=2;

	struct TestElement {
		uint8_t number;
		char *string;
	};
	struct TestElement originalElement = {
		.number = 8,
		.string = "ABCDEFG"
	};




	code = kaelTree_alloc(&tree, -1);
	if(code!=KAEL_SUCCESS){
		printf("Fail kaelTree_alloc\n");
	}
	kaelTree_setWidth(&tree, sizeof(originalElement));
	kaelTree_resize(&tree, elementCount);
	if(code!=KAEL_SUCCESS){
		printf("Fail kaelTree_resize\n");
	}
	kaelTree_reserve(&tree, reserveCount);
	if(code!=KAEL_SUCCESS){
		printf("Fail kaelTree_reserve\n");
	}


	struct TestElement *newPush = kaelTree_push(&tree, NULL);
	if(newPush==NULL){
		printf("Fail kaelTree_push\n");
	}
	code = kaelTree_pop(&tree);
	if(code!=KAEL_SUCCESS){
		printf("Fail kaelTree_pop\n");
	}


	struct TestElement *secondElement = kaelTree_insert(&tree, setElementIndex, NULL);
	if(secondElement == NULL ){
		printf("Fail kaelTree_insert\n");
	}
	kaelTree_set(&tree, setElementIndex, &originalElement);
	struct TestElement *secondElementGet = kaelTree_get(&tree, setElementIndex);
	if(secondElement != secondElementGet ){
		printf("Fail ptr kaelTree_get\n");
	}
	if(secondElement->string != secondElementGet->string ){
		printf("Fail ptr->string kaelTree_get\n");
	}
	kaelTree_pop(&tree);

	kaelTree_begin(&tree);
	kaelTree_back(&tree);
	uint16_t treeLength = kaelTree_length(&tree);
	if(treeLength!=elementCount){
		printf("Fail kaelTree_empty");
	}

	uint16_t isEmpty = kaelTree_empty(&tree);
	if(isEmpty){
		printf("Fail kaelTree_empty\n");
	}


	struct TestElement *currentElement = secondElement;
	kaelTree_prev(&tree, (void**)&currentElement);
	if(currentElement == NULL ){
		printf("Fail kaelTree_get\n");
	}
	kaelTree_next(&tree, (void**)&currentElement);
	if(currentElement != secondElement ){
		printf("Fail kaelTree_next\n");
	}


	struct TestElement *treeElement = kaelTree_get(&tree, setElementIndex);
	if(originalElement.number != treeElement->number){
		printf("Element at index %u should be identical to originalElement.\n",setElementIndex);
	}

	kaelTree_free(&tree);


	printf("kaelTree_functions_unit Done\n");
}