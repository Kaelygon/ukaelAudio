
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>




uint8_t unit_lcg24(uint8_t seed[3]){
	uint16_t carry = 205;
	
	carry+= seed[0];
	seed[0] = carry;
	
	carry+= seed[1] + (carry>>8);	// These two lines could be put inside a for loop
	seed[1] = carry;					// and iterated through seed[i], i=0 to array length
	
	seed[2]+= carry + (carry>>8);
		
	return seed[2];
}

void unit_printLcg24(FILE *fptr){
	uint8_t seed[3] = {0,0,0};
	for(uint32_t i=0; i<(uint32_t)pow(2, 8);i++){
		uint8_t num = unit_lcg24(seed);
		fwrite(&num,sizeof(uint8_t),1,fptr); //binary
	}
}





int main(int argc, char **argv){
	
	FILE *fptr; 
	fptr = fopen(argv[0],"r");
	if(fptr==NULL){printf("open failed\n"); abort();}
	
	unit_printLcg24(fptr);

	fclose(fptr); 
	return 0;
}






/*
*/