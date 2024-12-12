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

//---------
//1,  37,  57
 	
int main() {

	omp_set_num_threads(1); 

	const uint64_t checkCount=pow(2,24); //check up to this many iterations
	const uint64_t minPeriod=pow(2,23)-1; //pass test threshold

	const krand_t stateCount=2; //bytes in random state

	//randomness statistics
	const double maxZscore = 30.0; //any single rand test period max zscore passable, test will stop if this is exceeded 
	const double maxAvgZscore = 10.0; //print only values with lower average zscore than this
	const double minAvgZscore = 0; 
	
	const double randSCount = checkCount/32; //one rand test period sample count 

	const uint64_t firstPeriodCheck = checkCount; //preliminary period check length
	const uint8_t periodOnly = 0; //checks period up to firstPeriodCheck and skips rest of the tests

	const uint64_t modList[] = {3,7};
	const uint64_t modCount = sizeof(modList)/sizeof(uint64_t);
	const uint64_t diffModCount = 16;

	const uint8_t printDiffMod = 1;
	const uint8_t printAll = 1;

    const PrngCoeff coeff = {  1,   1,  1, kaelRandT_lcg, "kaelRandT_lcg"};

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
