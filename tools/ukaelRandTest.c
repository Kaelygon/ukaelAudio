




#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "../ukaelH/kmath.h"

#define TEST_NO 2
#define ELEMS ((uint32_t)1<<16)
int main() {
	srand(time(NULL));
	ukaelRdtscSeed();

	uint32_t list[ELEMS];
	for(uint32_t i=0;i<ELEMS;i++){
		list[i]=0;
	}

		uint32_t repeats=1000;
		uint32_t frequ=32768;
		

	//Finds two occurances of "segNum", and compares numbers at same position after segNum
	uint32_t segNum = 1; //where the tested run segment starts
	uint32_t const segLen=327680; //tested segment length after segNum
	int32_t pastSegNum[segLen];	//finds segNum and reads next values and compares them to next values after an other segNum
	pastSegNum[segLen-1]=-1; //-1=not filled yet
	uint32_t poinc=0; //past one array increment
	int8_t isPastSegNum=0; //past segNum
	uint32_t sameSegNum=0; //count of numbers with same position relative to segNum
	uint32_t mostSameSegNum=0; //most nums with -||-
	uint32_t idenSeg=0; //count of identical segments after segNum
	uint32_t testedSeg=0; //count of tested segments past segNum

	double percMean=0;	// percentage mean
	double pdelta=0; //average delta from previous value
	uint32_t prev=ELEMS/2;
    for (uint32_t i = 0; i < frequ*repeats; i++) {
		#if TEST_NO==0
			uint32_t buf=rand()%ELEMS;
			list[buf]++;
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
			percMean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			prev=buf;
		#elif TEST_NO==1
			prev=UKAEL_LCG.a%ELEMS;
			ukaelReseed();
			uint32_t buf=((uint32_t)(UKAEL_LCG.b<<16)|UKAEL_LCG.a)%ELEMS;
			list[buf]++;
			percMean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#elif TEST_NO==2
			prev=UKAEL_LCG.a%ELEMS;
			ukaelRdtscSeed();
			uint32_t buf=((uint32_t)(UKAEL_LCG.a<<16)|UKAEL_LCG.b)%ELEMS;
			list[buf]++;
			percMean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#elif TEST_NO==3
			prev=UKAEL_LCG.a%ELEMS;
			ukaelBadReseed();
			uint32_t buf=((uint32_t)(UKAEL_LCG.b<<16)|UKAEL_LCG.a)%ELEMS;
			list[buf]++;
			percMean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#endif
		if( buf==segNum ){
			isPastSegNum=1; //start storing or comparing segNum
			testedSeg++;
		}
		if(isPastSegNum){
			if(buf==segNum){continue;} //skip the one
			if(pastSegNum[segLen-1]==-1){//store segment if none is stored yet
				pastSegNum[poinc]=buf;
				poinc++;
				if(poinc>(segLen-1)){poinc=0;isPastSegNum=0;} 
			}else{//An other segNum was found
				if(pastSegNum[poinc]==buf){//compare number past previous segNum
					sameSegNum++;
				}
				poinc++;
				if(poinc>(segLen-1)){
					if(sameSegNum>=mostSameSegNum){ //set most same nums found in segment
						mostSameSegNum=sameSegNum;
					}
					if(sameSegNum>=segLen){ //identical segment count
						idenSeg++;
					}
					poinc=0;
					isPastSegNum=0;
					sameSegNum=0;
					pastSegNum[segLen-1]=-1;
				}
			}
		}
    }
	
	uint32_t noOccur=0; // count of numbers never occured
	uint32_t most=0; // most times some number occured
	uint32_t least=UINT32_MAX;
	uint32_t mostnum=0;	// the number that occured the most
	uint32_t leastnum=0;

	for(uint32_t i=0;i<ELEMS;i+=1){
		uint32_t buf = list[i];
		if(buf>most){
			most=buf;
			mostnum=i;
		}
		if(buf<least){
			least=buf;
			leastnum=i;
		}
		noOccur+=(buf==0);
		if(rand()>(INT32_MAX-INT32_MAX/2048)){ //print random samples
			printf("%u, %u\n",i,buf);
		}
	}

	//ideal averages with default values in comments. i'm not statistician
	printf("least[%u]:%u	mo[%u]:%u\n",leastnum,least,mostnum,most); // frequ*repeats/ELEMS=500 +-100
	printf("mo/le %f\n",(double)most/least); 		// 1.5 +-0.1
	printf("percMean %f\n",percMean); 				// 0.5 +-0.00001
	printf("pdelta %f\n",pdelta); 					// ELEMS/3=21845 +-3
	printf("noOccur %f\n",(double)noOccur/(ELEMS));	// 0.0
	printf("ELEMS %u\n",ELEMS);
	printf("samples %u\n",frequ*repeats);

	printf("\nSegments test\n");
	printf("testedSeg: %u\n",testedSeg); 			//frequ*repeats/ELEMS=500 +-100
	printf("mostSameSegNum: %u\n",mostSameSegNum); 	// 10 +- 5
	printf("idenSeg: %u\n",idenSeg); 				// astronomically rare to occur with random sequence

    return 0;
}
