#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h> 
#include <omp.h>

//#include "kaelygon/math/math.h"
//#include "kaelygon/math/rand.h"        
//#include "kaelygon/math/base256.h"

#include <omp.h>
#include "./kaelRandTesting.h"
#include "./k32/k32.h"

typedef uint8_t krand_t;

typedef enum {
	RAND_PASS = 0,
	FAIL_MOD = 1,
	FAIL_DIFF = 2,
	FAIL_BITONES = 3
}Kael_randFail;


//PCG search parameters
typedef struct {

	uint64_t minPeriod; //Print only params that exceeded this peripd
	uint64_t checkCount; //Quit all tests after this many iterations

	double maxAvgZscore; //max allowed zscore accounting all iterations 
    double minAvgZscore;

	double maxZscore; //max allowed zscore in every short period, length of randSCount 

	uint64_t randSCount; //rand checks sample count //The tests accumulate averages and reset after every randSCount
	uint8_t stateCount; //bytes of memory for PRNG states

	uint64_t firstPeriodCheck; //perliminary looping check will only check up to this value

    const uint64_t *modList;
    uint64_t modCount;
    uint64_t diffModCount;

    uint8_t periodOnly;
    uint8_t printAll;
    uint8_t printDiffMod;
}rlcg_args;


//Random valuation parameters
typedef struct{
    uint64_t inc; //iteration incrementor
    uint64_t currentValue; //current iteration output 
    uint64_t lastValue; //last iteration output

	uint8_t notRandom; //Flag to quit, see Kael_randFail above
	
    double avgZscore; //average of all tests
    uint64_t avgZscoreCount; //used to calculate average from test count
	double failedZscore; //Z score that exceeded the maximum 
}RandSearchIter;



// PRNG cycle period tuple
typedef struct {
	krand_t *state;
	uint64_t period;
    uint64_t stateCount;
}CycleTuple;

CycleTuple *kaelRandT_cycleTuple_new(uint8_t stateCount){
    CycleTuple *tuple = malloc(sizeof(CycleTuple));
    if(tuple==NULL){printf("kaelRandT_cycleTuple_new alloc failed"); abort();}

    krand_t *tmpState = malloc(stateCount*sizeof(krand_t));
    if(tmpState==NULL){printf("tuple->state alloc failed"); abort();}
    memset(tmpState,0,stateCount*sizeof(krand_t));
    
    tuple->state=tmpState;

    tuple->stateCount=stateCount;
    tuple->period=0;

    return tuple;
}

void kaelRandT_cycleTuple_del(CycleTuple **tuple){
    free((*tuple)->state);
    free(*tuple);
    *tuple=NULL;
}




//---------
//tools

uint64_t countOneBits(uint64_t num){
	uint64_t uCount;
	uCount = num - ((num >> 1) & 033333333333) - ((num >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

/*
    Calculate distance from mean in standard deviations, (Z score)
    of binomial distribution
    
    For example if we want z score of bit distribution 0s and 1s
    value = number of 0s or 1s
    sampleSize = sum of 0s and 1s
    outcomes = 2 // bit can be 0 or 1
*/
double calcZscore(const uint64_t comparedValue, const uint64_t sampleSize, const double outcomes){
        double expectProb = 1.0/outcomes; //probability of each outcome
		double mean = (double)(sampleSize) * expectProb; //expected mean of each outcome
		double delta = fabs(comparedValue - mean); //absolute distance from mean in units
		double stddev = sqrt(sampleSize * expectProb * (1.0 - expectProb)); //standard deviation of binomial distribution
		double Zscore = delta/stddev; //distance from mean in standard deviations
		return Zscore;
}

//Floyd's tort and hare
void kaelRandT_findCycle(CycleTuple *cycle, uint64_t maxCheckCount, const PrngCoeff coeff) {
    uint64_t stateCount = cycle->stateCount;
	kael32_t hare = {0};
	kael32_t tort = {0};

    cycle->period = UINT64_MAX;

    k32_seed(&hare,NULL);
    k32_seed(&tort,NULL);

	uint64_t count=0;
	do { //Main phase
		kaelRandT_base(&tort, coeff); //walk 1

		kaelRandT_base(&hare, coeff); //leap 2
		kaelRandT_base(&hare, coeff);

		count++;
		if(count>maxCheckCount){
			goto FINDCYCLE_FREE_LABEL; //no period 
		}
	} while (k32_cmp(&hare,&tort)!=128);

	//Find the position mu of first repetition.
	uint64_t mu=0;
    k32_seed(&tort,NULL);
	while (k32_cmp(&hare,&tort)!=128){
		kaelRandT_base(&tort, coeff);
		kaelRandT_base(&hare, coeff);
		mu++;
		if(mu>maxCheckCount){
            printf("Invalid mu\n");
			goto FINDCYCLE_FREE_LABEL; 
		}
	}

	//Convert offset to starting state
	for(uint64_t i=stateCount-1; i!=0; i--){
		if(mu==0){break;}
		cycle->state[i]=mu&0xFF;
		mu>>=CHAR_BIT;
	}

	cycle->period=0;
	do { //iterate till back to the starting state
		kaelRandT_base(&tort, coeff);
		cycle->period++;
		if(cycle->period>maxCheckCount){
            printf("Invalid period\n");
			break; 
		}
	} while (k32_cmp(&hare,&tort)!=128);
	
    FINDCYCLE_FREE_LABEL:
	return;
}




//--------- Randomness test



// Take modulus and compare remained distribution, this can be done for multiple values
typedef struct {
    uint64_t modTestCount;
    uint64_t *modList;
    uint64_t **remCount;
}RandModTest;

RandModTest *kaelRandT_manyMod_new(const uint64_t *constModList, const uint64_t modCount){

    RandModTest *mt = malloc(sizeof(RandModTest));
    if(mt==NULL){printf("kaelRandT_manyMod_new alloc failed"); abort();}

	//repetition checks for various modulo
    mt->modTestCount=modCount;

    uint64_t *tmpModList = (uint64_t *)malloc( sizeof(uint64_t)* mt->modTestCount );
    if(tmpModList==NULL){printf("modList alloc failed"); abort();}
    mt->modList=tmpModList;

	uint64_t **tmpTemCount = malloc(sizeof(uint64_t *) * mt->modTestCount); //each modulus has a counter
    if(tmpTemCount==NULL){printf("remCount alloc failed"); abort();}
	for(uint64_t i=0; i < mt->modTestCount; i++){
        mt->modList[i]=constModList[i];
		tmpTemCount[i] = calloc(constModList[i], sizeof(uint64_t));//counters for each remainder
		if(tmpTemCount[i]==NULL){printf("alloc remCount[] failed"); abort();}
	}
    mt->remCount=tmpTemCount;

    return mt;
}

void kaelRandT_manyMod_del(RandModTest **mt){
	for(uint64_t i=0; i < (*mt)->modTestCount; i++){
        free((*mt)->remCount[i]);
    }
    free((*mt)->modList);
    free((*mt)->remCount);
    free(*mt);
    *mt=NULL;
}

void kaelRandT_manyMod(const rlcg_args arg, RandModTest *mt, RandSearchIter *params, const uint64_t runTest ){
    if(mt==NULL){return;}
    //modulus distribution check
    for (uint64_t i = 0; i < mt->modTestCount; i++) { //count rem of each mod
        uint64_t newMod = params->currentValue % mt->modList[i]; 
        mt->remCount[i][newMod]++; //count up the remainder
    }
    if( runTest ){ //calculate z score
        for (uint64_t i = 0; i < mt->modTestCount; i++) { //check each modulus
            for(uint64_t j = 0; j < mt->modList[i]; j++){ //check each remainder
                double Zscore = calcZscore( mt->remCount[i][j], arg.randSCount, mt->modList[i] );
                if (Zscore > arg.maxZscore) {
                    params->failedZscore=Zscore;
                    params->notRandom=FAIL_MOD;
                }
                
                params->avgZscoreCount++;
                params->avgZscore=(Zscore * (params->avgZscoreCount-1) + params->avgZscore) / params->avgZscoreCount;
                mt->remCount[i][j]=0; //reset counter
            }
        }
    }
}





//------ Take modulus of the difference of current and last value and compare their distribution

typedef struct {
    uint64_t diffModulus; //Number that the remainder will be divided by
    uint64_t *diffCounters; //Counter for each difference remainder
}RandDiffTest;

RandDiffTest *kaelRandT_diffMod_new(const uint64_t diffModulus){
    RandDiffTest *dt = malloc(sizeof(RandDiffTest));
    if(dt==NULL){printf("kaelRandT_diffMod_new alloc failed"); abort();}

    dt->diffModulus = diffModulus;
    uint64_t *tmpCounters = calloc(diffModulus, sizeof(uint64_t));
    if(tmpCounters==NULL){printf("diffCounters alloc failed"); abort();}
	memset(tmpCounters,0,sizeof(uint64_t)*diffModulus);

    dt->diffCounters=tmpCounters;
    return dt;
}

void kaelRandT_diffMod_del(RandDiffTest **dt){
	free((*dt)->diffCounters);
    free(*dt);
    *dt=NULL;
}

void kaelRandT_diffMod(const rlcg_args arg, RandDiffTest *dt, RandSearchIter *params, const uint8_t runTest ){
    if(dt==NULL){return;}
    krand_t newDiff = (params->currentValue - params->lastValue); //How much was added to reach current value
    newDiff %= dt->diffModulus;
    dt->diffCounters[newDiff]++;
    params->lastValue = params->currentValue;
    if( runTest ){
        for (uint64_t i = 0; i < dt->diffModulus; i++) {
            double Zscore = calcZscore(dt->diffCounters[i], arg.randSCount, dt->diffModulus);
            if (Zscore > arg.maxZscore) {
                params->failedZscore=Zscore;
                params->notRandom=FAIL_DIFF;
            }

            params->avgZscoreCount++;
            params->avgZscore=(Zscore * (params->avgZscoreCount-1) + params->avgZscore) / params->avgZscoreCount;
            dt->diffCounters[i]=0; //reset counter
        }
    }
}






//------ Zeros and ones bit distribution test

typedef struct {
    uint64_t onesCount; 
}RandBitDistTest;

RandBitDistTest *kaelRandT_bitCounts_new(){
    RandBitDistTest *bt = calloc(1, sizeof(RandBitDistTest));
    if(bt==NULL){printf("kaelRandT_bitCounts_new alloc failed"); abort();}
    return bt;
}

void kaelRandT_bitCounts_del(RandBitDistTest **bt){
    free(*bt);
    *bt=NULL;
}

void kaelRandT_bitDist(const rlcg_args arg, RandBitDistTest *bt, RandSearchIter *params, const uint64_t runTest ){
    if(bt==NULL){return;}
    bt->onesCount += countOneBits(params->currentValue);
    if( runTest ){
        double outcomes = 2; //bit can be 0 or 1
        double totalBits = arg.randSCount*sizeof(krand_t)*CHAR_BIT;
        double Zscore = calcZscore(bt->onesCount, totalBits, outcomes);
        if (Zscore>arg.maxZscore) {
            params->failedZscore=Zscore;
            params->notRandom=FAIL_BITONES;
        }

        params->avgZscoreCount++;
        params->avgZscore=(Zscore * (params->avgZscoreCount-1) + params->avgZscore) / params->avgZscoreCount;
        bt->onesCount=0; //reset counter
    }
}


void kaelRandT_printArray(krand_t *num, size_t count, uint8_t printBrackets){
    if(printBrackets){
        printf("{");
    }
    for(uint8_t i=0; i<count; i++){
        uint8_t maxDigits = log10((double)((krand_t)~0))+1; //max uint digits
        uint8_t numDigits = log10((double)(num[i] + (num[i]==0)))+1;
        uint8_t spaceCount = maxDigits - numDigits;
        //Add padding depending how many digits the number has 
        for(uint8_t j=0; j<spaceCount; j++ ){
            printf(" ");
        }
        printf("%u",num[i]);
        if(i!=(uint64_t)(count-1)){printf(", ");}
    }
    if(printBrackets){
        printf("}, ");
    }
}

//------ Run randomness tests 
//maybe this could take *kaelStr instead of print
void kaelRandT_printPRNGResult( const rlcg_args arg, const RandSearchIter params, const CycleTuple *startState, const PrngCoeff coeff, const RandDiffTest *diffTest){
    #define PTR_TO_CHAR(ptr)(#ptr)
    
    if(arg.printDiffMod){
        printf("Difference mod %lu congruence distribution\n",diffTest->diffModulus);
        for(uint64_t i=0; i<diffTest->diffModulus; i++){
            printf("%lu:%lu\n", i, diffTest->diffCounters[i]);
        }
    }

    //print parameters
    krand_t printCoeffs[3] = {coeff.shift, coeff.mul, coeff.add};
    printf("{");
    kaelRandT_printArray(printCoeffs,3,0);
    printf(", %s, \"%s\"",coeff.name,coeff.name);
    printf("}, ");
    printf("//");
    
    //zscore
    printf(" Z:%.2f - ", params.avgZscore);

    //start state
    kaelRandT_printArray(startState->state, startState->stateCount,1);
    printf(" - ");

    //period
    printf("fail:%u Z:%.2f, at %lu - ", params.notRandom, params.failedZscore, params.inc);
    if(startState->period==UINT64_MAX){ //No period found up to arg.firstPeriodCheck
        printf("period ?\t - \t" );
    }else{
        printf("period %lu - \t", startState->period );
    }

    //print sample
    kael32_t sample = {0};
    memcpy(sample.s, startState->state, KAEL32_BYTES);
    for(uint64_t i=0; i<12; i++){ 
        uint64_t iterValue = kaelRandT_base(&sample, coeff); 
        printf("%lu ",iterValue);
    }

    printf("\n");
}

//------
//execute various tests and print results
uint8_t kaelRandT_runRandTests(const rlcg_args arg, const PrngCoeff coeff){ 
	uint8_t passed=0;

	RandSearchIter params;
    memset(&params,0,sizeof(RandSearchIter));

    //store starting state as it may be printed
	CycleTuple *startState = kaelRandT_cycleTuple_new(arg.stateCount);
    //kaelRandT_findCycle returns -1 if it didn't find loop
    kaelRandT_findCycle(startState, arg.firstPeriodCheck, coeff);

    //if looped before minimum accepted period
	if( (startState->period < arg.minPeriod) && !arg.printAll ){ 
        kaelRandT_cycleTuple_del(&startState);
        return 0;
	}

	//Init randomness tests
    RandModTest *modTest = kaelRandT_manyMod_new(arg.modList, arg.modCount);
    RandDiffTest *diffTest = kaelRandT_diffMod_new(arg.diffModCount);
	RandBitDistTest *bitDistTest = kaelRandT_bitCounts_new();
    kael32_t randState = {0};

    //full period was tested
	uint8_t periodIsTested = 0;
    if( arg.firstPeriodCheck >= arg.minPeriod ){
        periodIsTested=1; 
    }

    //no use of checking repeating period
	uint64_t stopCount = startState->period < arg.checkCount ? startState->period : arg.checkCount; 

    if(arg.periodOnly){
        params.inc=stopCount-1;
    }
	
    //copy for iterations
    memcpy(&randState.s, &startState->state, KAEL32_BYTES );

	while(1){ //BOF Random test loop
		params.inc++;

		if( ( params.inc >= stopCount) || (params.notRandom && !arg.printAll) ){ //break if inc beyond stopping point or some test failed

            //printing conditions
            //zscore within min and max
            uint8_t isValidZscore = (params.avgZscore < arg.maxAvgZscore) && (params.avgZscore >= (arg.minAvgZscore) );
            //Ignore zscore
            if(arg.periodOnly){
                isValidZscore=1;
            }
            uint8_t isLongEnoughPeriod = (params.inc >= arg.minPeriod);
            
			if( (isLongEnoughPeriod && isValidZscore) || arg.printAll ){ //print
				#pragma omp critical
				{
                    kaelRandT_printPRNGResult(arg, params, startState, coeff, diffTest);
                    passed = 1;
				}
			}

			break;
		}

		//finding cycle is expensive so do it only after few initial tests
		if(periodIsTested==0 && params.inc > arg.firstPeriodCheck){ 
			kaelRandT_findCycle(startState, arg.checkCount, coeff);
			periodIsTested=1;
			if(startState->period<arg.minPeriod){
				params.notRandom=1;
			}
	        //recalculate stopping point
	        stopCount = startState->period < arg.checkCount ? startState->period : arg.checkCount; 
		}

		params.currentValue = kaelRandT_base(&randState, coeff); //iterative value

		//randomness checks

        //Each test accumulates data and runs tests every 'randSCount'th iteration 
		uint8_t runTest = (params.inc % arg.randSCount) == 0 ;

		//modulus distribution check
		kaelRandT_manyMod(arg, modTest, &params, runTest);

		//Difference distribution
        kaelRandT_diffMod(arg, diffTest, &params, runTest);
        
		//bit 0s and 1s ratio
        kaelRandT_bitDist(arg, bitDistTest, &params, runTest);

		

	} //EOF randomness test loop

    //cleanup
    kaelRandT_manyMod_del(&modTest);
    kaelRandT_diffMod_del(&diffTest);
	kaelRandT_bitCounts_del(&bitDistTest);

    kaelRandT_cycleTuple_del(&startState);

	return passed;
}
