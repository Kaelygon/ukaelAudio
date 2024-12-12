// 
//These tests certainly don't prove that the generated numbers are random
//There are many patterns that go undetected, but this is a great fast tool 
//to find period and prune out obviously repeating patterns
/*
The final implementation is significantly more simplified, hende
KaelRandT = KaelRand Testing
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

	omp_set_num_threads(24); 

	const uint64_t checkCount=pow(2,24); //check up to this many iterations
	const uint64_t minPeriod=pow(2,23.9)-1; //pass test threshold
	uint64_t totalCandies=0;

	const krand_t minShift=1;
	const krand_t maxShift=8;
	const krand_t shiftInc=1;

	const krand_t minMul=1;
	const krand_t maxMul=100;
	const krand_t mulInc=1;

	const krand_t minAdd=1;
	const krand_t maxAdd=100;
	const krand_t addInc=1;

	const krand_t stateCount=2; //bytes in random state

	//randomness statistics
	const double maxZscore = 100.0; //any single rand test period max zscore passable, test will stop if this is exceeded 
	const double maxAvgZscore = 10.0; //print only values with lower average zscore than this
	const double minAvgZscore = 0.0; 
	
	const double randSCount = minPeriod; //one rand test period sample count 

	const uint64_t firstPeriodCheck = checkCount; //preliminary period check length
	const uint8_t periodOnly = 0; //checks period up to firstPeriodCheck and skips rest of the tests

	const uint64_t modList[] = {3,7};
	const uint64_t modCount = sizeof(modList)/sizeof(uint64_t);
	const uint64_t diffModCount = 16;

	const uint8_t printAll = 0; //print even failed tests

	printf("// {shift, mult, add} // Z:score - startState - fail:ID Z:score - period - sample\n");
	printf("krand_t sft_mul_add[][3] = {\n");

	for(krand_t shift=minShift;shift<maxShift;shift+=shiftInc){
		//OMP is unhappy with irregular ranges
		krand_t ompMaxMul = (maxMul-minMul)/mulInc;
		krand_t ompMaxAdd = (maxAdd-minAdd)/addInc;

   		#pragma omp parallel for collapse(2) schedule(dynamic)
		for(krand_t mul=0; mul<ompMaxMul; mul++){
			for(krand_t add=0; add<ompMaxAdd; add++){

				krand_t newMul = mul*mulInc+minMul;	
				krand_t newAdd = add*addInc+minAdd;			

				const PrngCoeff coeff = {
					.shift 				= shift	 			,
					.mul 				= newMul			,
					.add 				= newAdd			,
					.oper				= kaelRandT_rorr	,
					.name				="kaelRandT_rorr"
				};

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
					.printAll			= printAll			,
				};
				uint8_t passed = kaelRandT_runRandTests(mainArgs,coeff);
				#pragma omp critical
				if(passed){
					totalCandies++;
				}
			}
		}
	}
	printf("};\n");
	printf("// Total: %lu\n",totalCandies);

	return 0;
}
