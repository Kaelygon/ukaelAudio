// 
/*
Verify single unit
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h> 
#include <omp.h> //Valgrind will claim OMP leaking memory


#include "./include/kaelRandTesting.h"
#include "./include/kaelRandTrials.h"
#include "../../include/kaelygon/math/k32.h"

//---------
//1,  37,  57
 	
int main() {

	omp_set_num_threads(0); 
	
	const uint64_t checkCount=pow(2,24); //check up to this many iterations
	const uint64_t minPeriod=pow(2,24)-1; //pass test threshold

	const krand_t stateCount=KAEL32_BYTES; //bytes in random state

	//randomness statistics
	const double maxZscore = 10.0; //any single rand test period max zscore passable, test will stop if this is exceeded 
	const double maxAvgZscore = 10.0; //print only values with lower average zscore than this
	const double minAvgZscore = 0; 
	
	const double randSCount = checkCount/2; //one rand test period sample count 

	const uint64_t firstPeriodCheck = checkCount; //preliminary period check length
	const uint8_t periodOnly = 0; //checks period up to firstPeriodCheck and skips rest of the tests

	const uint64_t modList[] = {3,7};
	const uint64_t modCount = sizeof(modList)/sizeof(uint64_t);
	const uint64_t diffModCount = 256;

	const uint8_t printDiffMod = 1;
	const uint8_t printAll = 1;

	const PrngCoeff coeff = (PrngCoeff) {  1,  89, 57, kaelRandT_lcg, "kaelRandT_lcg"}; //full period

    const rlcg_args mainArgs = (rlcg_args){
        .minPeriod 			= minPeriod			,
        .checkCount 		= checkCount		,
        .maxAvgZscore		= maxAvgZscore		, 
        .minAvgZscore		= minAvgZscore		,
        .maxZscore 			= maxZscore			,
        .randSCount 		= randSCount		,
        .stateCount			= stateCount		,
        .firstPeriodCheck 	= firstPeriodCheck	,
        .modList			= modList			,
        .modCount			= modCount			,
        .diffModCount		= diffModCount		,
        .periodOnly			= periodOnly		,
        .printAll		    = printAll		    ,
        .printDiffMod       = printDiffMod      ,
    };
    uint8_t passed = kaelRandT_runRandTests(mainArgs,coeff);
                
	printf("passed: %u\n",passed);

	return 0;
}
