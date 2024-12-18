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



void unitTest_runTests(){
	void(*unitTest_func[])() = {
		kaelTerminal_unit	,
		kaelString_unit	,
		kaelTree_unit		,
		kaelRand_unit		,
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	for(uint16_t i=0; i<unitTestCount; i++){
		unitTest_func[i]();
		#if KAEL_DEBUG==1
			kaelDebug_printInfoStr();
		#endif
	}
}


int main(){
	#if KAEL_DEBUG==1
		kaelDebug_allocGlobal();
	#endif

	unitTest_runTests();

	#if KAEL_DEBUG==1
		kaelDebug_freeGlobal();
	#endif
	
   return 0;
}


