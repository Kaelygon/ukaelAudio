/**
 * @file kaelMemUnit.h
 * 
 * @brief test kaelMem
 */

#pragma once

#include <stdio.h>
#include <stdint.h>


#include "kaelygon/treeMem/treeMem.h"
#include "kaelygon/math/math.h"
#include "kaelygon/math/rand.h"
#include "kaelygon/kaelMacros.h" 


void kaelMem_unit() {

	uint16_t outNum=0;

	KaelMem *forest = kaelMem_alloc(sizeof(KaelMem)); //create new "object" forest
	kaelMem_setHeight(forest,2); // nested, forest->Data = (KaelMem *)tree -> (KaelMem)branch -> (uint16_t)leaf

	KaelMem *tree = kaelMem_push(forest,NULL); //push a tree to the forest
	kaelMem_setSize(tree,sizeof(KaelMem));
	kaelMem_setHeight(tree,1);

	KaelMem *treeTwo = kaelMem_push(forest,NULL); //push an other tree, but this time uninitialized
	kaelMem_setHeight(treeTwo,1);

	uint16_t leaves = 3;
	uint16_t branches = 4; 

	printf("\n--Writing---\n");

	//generate the tree
	for(uint16_t i=0;i<branches;i++){
		KaelMem *branch = kaelMem_push(tree,NULL); //add empty branches and get its pointer
		if(branch==NULL){break;}
		printf("\nbranch %u\n",i);
		kaelMem_setSize(branch,sizeof(uint16_t)); //set leaf byte width
		for(uint16_t j=0;j<leaves;j++){ //add leaves
			uint16_t randNum=j+i*leaves;
			randNum=kaelRand_u16lcg(randNum);
			outNum+=randNum;
			printf("leaf %u: %u\n", j, randNum);
			KaelMem* leafPtr = kaelMem_push(branch,&randNum);
			if(leafPtr==NULL){break;}
		}
	}
	
	KaelMem *firstTree = (KaelMem *)kaelMem_get(forest,0);
	kaelMem_pop(firstTree); //delete last branch of the first tree
	KaelMem *secondBranch = kaelMem_get(firstTree,1); 
	kaelMem_pop(secondBranch); //delete last leaf of the first tree, second branch

	uint16_t *leafItem = kaelMem_get(secondBranch,0); //set value by pointer
	if(leafItem!=NULL){
		*leafItem=22222;
	}
	secondBranch=NULL;
	leafItem=NULL;

	uint16_t leafValue=11111;
	firstTree = kaelMem_begin(forest);
	KaelMem *firstBranch = kaelMem_begin(firstTree);
	kaelMem_set(firstBranch,1,&leafValue); //set value by element

	firstBranch=NULL;
	firstTree=NULL;
	
	printf("\n--Reading---\n");

	//read every element of the tree
	for(uint16_t i=0;i<kaelMem_count(tree);i++){
		KaelMem *branch = kaelMem_get(tree,i);
		printf("\nbranch %u\n",i);
		for(uint16_t j=0;j<kaelMem_count(branch);j++){ //read leaves
			uint16_t *randNum=kaelMem_get(branch,j);
			outNum+=*randNum;
			printf("leaf %u: %u\n", j, *randNum);
		}
	}

	//This will recursively delete everything if height and sizes are set correctly
	kaelMem_free(&forest);

	printf("%u\n",outNum);

	printf("kaelMem_unit Done\n");	
}



