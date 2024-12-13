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


int main(){
	void(*unitTest_func[])() = {
		kaelTerminal_unit	,
		kaelTree_unit		,
		kaelRand_unit	,
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	#if KAEL_DEBUG==1
		uint8_t code = kaelDebug_allocGlobal();
		if(code){printf("Error in global debug string.\n"); return 0;}
	#endif

	for(uint16_t i=0; i<unitTestCount; i++){
		unitTest_func[i]();
		#if KAEL_DEBUG==1
			code = kaelDebug_printInfoStr();
			if(code){printf("Error printing debug\n");}
		#endif
	}

	#if KAEL_DEBUG==1
		kaelDebug_freeGlobal();
	#endif
   return 0;
}


