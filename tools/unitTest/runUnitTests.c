/**
 * @file runUnitTests.c
 * 
 * @brief Run all unit tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "kaelygon/global/kaelMacros.h"

#include "./include/kaelTreeMemUnit.h"
#include "./include/kaelRandUnit.h"
#include "./include/kaelTerminalUnit.h"
#include "./include/kaelStringUnit.h"
#include "./include/krleConvert.h"

//Some tests result is irrelevant as there's no checks of the result correctness.  
//Mainly these made to find any unintentional NULL values (generated/kael.log) or valgrind errors
//Could be better, but I rather spend the time writing the actual program than testing 
void unitTest_runTests(){
	void(*unitTest_func[])() = {
		kaelTerminal_unit, //Test clock in terminal loop example. Fails if kaelClock deviates too much from std clock(). 
		kaelTree_drawSquares_unit, //Print ascii squares stored in branched kaelTree.
		kaelTree_functions_unit, //Good test. Stores element, iterate, insert and compare if the data and pointers are unchanged.
		kaelString_unit,
		kaelRand_unit,
		krleTGA_unit, //Good test. Convert TGA->KRLE->TGA twice and compare the results
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	for(uint16_t i=0; i<unitTestCount; i++){
		printf("\n------ Test %u ------\n\n", i);
		unitTest_func[i]();
	}
}


int main(int argc, char *argv[]){
	if(argc>0){
		kaelDebug_alloc(argv[0]);
	}

	unitTest_runTests();

	kaelDebug_free();
	
   return 0;
}


