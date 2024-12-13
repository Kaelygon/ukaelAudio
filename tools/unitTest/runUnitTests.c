/**
 * @file runUnitTests.c
 * 
 * @brief Run all unit tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "kaelygon/kaelMacros.h"
#include "./include/kaelTreeMemUnit.h"
#include "./include/kaelRandUnit.h"
#include "./include/kaelTerminalUnit.h"

#include "./include/unitTest.h"

int main(){

	void(*unitTest_func[])() = {
		kaelTerminal_unit	,
		kaelMem_unit		,
		kaelRand_unit		,
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	unitTest_allocGlobalStr(255);

	for(uint16_t i=0; i<unitTestCount; i++){
		unitTest_func[i]();
		unitTest_printDebugStr();
	}

	unitTest_freeGlobalStr();
    return 0;
}