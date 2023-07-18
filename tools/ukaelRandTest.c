




#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "../ukaelH/kmath.h"

#define TEST_NO 1
#define ELEMS (1<<16)
int main() {
	srand(time(NULL));
	ukaelRdtscSeed();

	uint32_t list[ELEMS];
	for(uint32_t i=0;i<ELEMS;i++){
		list[i]=0;
	}

		uint32_t repeats=1000;
		uint32_t frequ=32768;
		
	KAENTROPY.b+=KAENTROPY.b * 347 + 34111;
	KAENTROPY.a<<=7;

	double percmean=0;	// percentage mean
	double pdelta=0; //average delta from previous value
	uint16_t prev=rand()%ELEMS;
    for (uint32_t i = 0; i < frequ*repeats; i++) {
		#if TEST_NO==0
			uint16_t buf=rand()%ELEMS;
			list[buf]++;
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
			percmean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			prev=buf;
		#elif TEST_NO==1
			prev=KAENTROPY.a%ELEMS;
			ukaelReseed();
			uint16_t buf=((uint16_t)KAENTROPY.a)%ELEMS;
			list[buf]++;
			percmean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#elif TEST_NO==2
			prev=KAENTROPY.a%ELEMS;
			ukaelRdtscSeed();
			uint16_t buf=((uint16_t)KAENTROPY.a)%ELEMS;
			list[buf]++;
			percmean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#elif TEST_NO==3
			prev=KAENTROPY.a%ELEMS;
			ukaelBadReseed();
			uint16_t buf=((uint16_t)KAENTROPY.a)%ELEMS;
			list[buf]++;
			percmean+= (double)(buf)/(frequ*repeats)/(ELEMS-1);
			pdelta+=(double)abs(prev-buf)/(frequ*repeats);
		#endif
    }
	
	uint16_t noOccur=0; // count of numbers never occured
	uint32_t most=0; // most times some number occured
	uint32_t least=UINT32_MAX;
	uint32_t mostnum=0;	// the number that occured the most
	uint32_t leastnum=0;

	for(uint32_t i=0;i<ELEMS;i+=1){
		if(list[i]>most){
			most=list[i];
			mostnum=i;
		}
		if(list[i]<least){
			least=list[i];
			leastnum=i;
		}
		noOccur+=(list[i]==0);
		if(rand()>(INT32_MAX-INT32_MAX/2048)){ //print random samples
			printf("%d, %d\n",i,list[i]);
		}
	}

	//ideal average values commented for 32768000 of 16bit samples
	printf("least[%d]:%d	mo[%d]:%d\n",leastnum,least,mostnum,most); // frequ*repeats/ELEMS=500 +-100
	printf("mo/le %f\n",(double)most/least); 		// 1.5 +-0.1
	printf("percmean %f\n",percmean); 				// 0.5 +-0.00001
	printf("pdelta %f\n",pdelta); 					// ELEMS/3=21845 +-3
	printf("noOccur %f\n",(double)noOccur/(ELEMS));	// 0.0
	printf("samples %d\n",frequ*repeats);

    return 0;
}
