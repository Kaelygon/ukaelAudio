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


void unitTest_runTests(){
	void(*unitTest_func[])() = {
		kaelTerminal_unit,
		kaelTree_drawSquares_unit,
		kaelTree_functions_unit,
		kaelString_unit,
		kaelRand_unit,
		krleTGA_unit,
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	for(uint16_t i=0; i<unitTestCount; i++){
		printf("\n------ Test %u ------\n\n", i);
		unitTest_func[i]();
	}
}


int main(int argc, char *argv[]){
	#if KAEL_DEBUG==1
		if(argc>0){
			kaelDebug_alloc(argv[0]);
		}
	#endif

	unitTest_runTests();

	#if KAEL_DEBUG==1
		kaelDebug_free();
	#endif
	
   return 0;
}


