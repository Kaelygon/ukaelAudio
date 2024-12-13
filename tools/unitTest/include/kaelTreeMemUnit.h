/**
 * @file kaelTreeUnit.h
 * 
 * @brief test kaelTree
 */

#pragma once

#include <stdio.h>
#include <stdint.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/treeMem/treeMem.h"
#include "kaelygon/math/math.h"
#include "kaelygon/math/rand.h"


void kaelTree_unit() {

	uint16_t branchCount = 16;
	uint16_t leafCount = 16; 

	uint16_t outNum=0;

	KaelTree forest; //create forest "instance"
	uint8_t err = kaelTree_alloc(&forest,sizeof(KaelTree)); //allocate
	if(err){printf("Failed to allocate forest.");}

	kaelTree_setHeight(&forest,2); // nested, forest->Data = (KaelTree *)tree -> (KaelTree)branch -> (uint16_t)leaf

	KaelTree *tree = kaelTree_push(&forest,NULL); //push a tree to the forest
	kaelTree_setHeight(tree,1);
	kaelTree_setWidth(tree,sizeof(KaelTree));

	KaelTree *treeTwo = kaelTree_push(&forest,NULL); //push an other tree, but this time height 0
	//kaelTree_setHeight(treeTwo,1); //Wrong height like this will result in error or double free
	kaelTree_setWidth(treeTwo,sizeof(uint8_t));
	uint8_t oneLeaf = 123;
	kaelTree_push(treeTwo,&oneLeaf); //just one leaf

	printf("\n--Writing---\n");

	//generate the tree
	for(uint16_t i=0;i<branchCount;i++){
		KaelTree *branch = kaelTree_push(tree,NULL); //add empty branchCount and get its pointer. 
		if(branch==NULL){break;}
		kaelTree_setWidth(branch,sizeof(uint16_t)); //set leaf byte width // setSize
		kaelTree_resize(branch,leafCount); //pre-allocate leaves, setWidth resize order doesn't matter

		//printf("\nbranch %u\n",i);
		for(uint16_t j=0;j<leafCount;j++){ //add leafCount
			uint16_t randNum=j+i*leafCount;
			randNum=kaelRand_u16lcg(randNum);
			outNum+=randNum;
			//printf("leaf %u: %u\n", j, randNum);
			kaelTree_set(branch,j,&randNum); //Since we resized, we set instad of push. Pushing would append elements after the resize
		}
	}
	printf("%u\n",outNum);
	outNum=0;

	
	KaelTree *firstTree = (KaelTree *)kaelTree_get(&forest,0);
	kaelTree_pop(firstTree); //delete last branch of the first tree
	KaelTree *secondBranch = kaelTree_get(firstTree,1); 
	kaelTree_pop(secondBranch); //delete last leaf of the first tree, second branch

	uint16_t *leafItem = kaelTree_get(secondBranch,0); //set value by pointer
	if(leafItem!=NULL){
		*leafItem=22222;
	}
	secondBranch=NULL;
	leafItem=NULL;

	uint16_t leafValue=11111;
	firstTree = kaelTree_begin(&forest);
	KaelTree *firstBranch = kaelTree_begin(firstTree);
	kaelTree_set(firstBranch,1,&leafValue); //set value by element

	firstBranch=NULL;
	firstTree=NULL;
	
	printf("\n--Reading---\n");

	//read every element of the tree
	for(uint16_t i=0;i<kaelTree_count(tree);i++){
		KaelTree *branch = kaelTree_get(tree,i);
		//printf("\nbranch %u\n",i);
		for(uint16_t j=0;j<kaelTree_count(branch);j++){ //read leafCount
			uint16_t *randNum=kaelTree_get(branch,j);
			outNum+=*randNum;
			//printf("leaf %u: %u\n", j, *randNum);
		}
	}

	//This will recursively delete everything if height and sizes are set correctly
	kaelTree_free(&forest);

	printf("%u\n",outNum);

	printf("kaelTree_unit Done\n");	
}



