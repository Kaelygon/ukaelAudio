
/**
 * While possible, I don't think I am ready to create a new pointer system in 16-bits
 * It's going to be just assembly but worse 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>
#include "kaelygon/global/kaelMacros.h"


// ------ Memory initialization ------

typedef struct{
	uint16_t bank;
	uint16_t addr;
}KaelSys_ptr;

/**
 * Bank 0, header and system read only 64 Kib
 * bank 1, instruction cache
 * Bank 2-6, pointer Table 256 Kib
 * Bank 7, device feeds
 */

//Seg table pointer indices
//e.g. print feed pointer is stored in {0x0002, KS_PRINT_FEED} which points to {0x0007, 0x0100},
//This list stores only system initialized read only pointers
//
//The emulated program can request allocations which pointers are appended to this and KS_SEG_TOP is incremented
typedef enum{
	KS_SEG_TABLE, //segTable location in memRaw
	KS_SEG_TOP, //segTable last element index
	
	KS_REG, //Registers
	
	KS_CLOCK_HIGH, //32768hz clock 
	KS_CLOCK_LOW,

	KS_PRINT_FEED, 
	KS_AUDIO_FEED,

	KS_SEG_COUNT, //enum count
}KaelSys_segIndex;

//------ pointer table ------
// pointers to devices and stored data in memRaw
//Initial segment table that will be copied to memRaw at segTable[0]
//similar to stack
const KaelSys_ptr segTable[] = {
	{0x0002, 0x0000},	// KS_SEG_TABLE address to itself
	{0x0000, KS_SEG_COUNT}, //First 4-byte is segTable top index

	{0x0001, 0x0000}, // KS_REG

	//system
	{0x0000, 0x0010}, // KS_CLOCK_HIGH
	{0x0000, 0x0011}, // KS_CLOCK_LOW

	//Feeds. Emulator will clear these feeds periodically
	{0x0007, 0x0100}, // KS_PRINT_FEED
	{0x0007, 0x0200}, // KS_AUDIO_FEED
};
uint32_t segTableSize = sizeof(segTable)/sizeof(segTable[0]);

//Header starting at 0x1 after NULL
//This would store all the information critical for the program to understand its environment
typedef enum{
	KH_NULL,
	KH_SEG_TABLE,
}KaelSys_headerIndex;

const uint8_t headerData[] = {
	0x0, //NULL
	0x0002, 0x0000, //segTable[KS_SEG_TABLE].bank, segTable[KS_SEG_TABLE].addr
};
uint32_t headerSize = sizeof(headerData)/sizeof(headerData[0]);

void kaelSys_memRawInit(uint8_t* memRaw, const uint8_t *headerData, uint32_t headerSize, const KaelSys_ptr *segTable, uint32_t segCount){
	if(NULL_CHECK(memRaw)){
		return;
	};
	memcpy(memRaw, headerData, headerSize);
	
	//offset to header table
	memRaw = memRaw + (segTable[0].bank*UINT16_MAX+segTable[0].addr);	
	size_t ptrSize = sizeof(segTable[0]);

	for(uint32_t i=0; i<segCount; i++){
		memcpy(memRaw+i*ptrSize, segTable, ptrSize);
	}

}



//------ Emulation ------

typedef enum{
	KS_QUIT,
	
	KS_ALLOC,

	KS_STORE,
}KaelSys_Instr;

uint8_t *memRaw = NULL;
uint8_t quitFlag = 0;

void *KaelSys_getAddr(KaelSys_ptr kptr){
	return memRaw + UINT16_MAX * kptr.bank + kptr.addr;
}

void kaelSys_runInstr(uint16_t instr, uint16_t arg1, uint16_t arg2){
	if(NULL_CHECK(memRaw)){return;}
	switch(instr){
		case KS_QUIT:
			quitFlag=1;
			return;
			
		case KS_ALLOC:
			//Just simple example, we aren't keeping track allocation size yet
			//First element is the segment table top incrementor
			uint32_t *topInc = KaelSys_getAddr(segTable[KS_SEG_TOP]); 
			*topInc+=1; 

			//"make allocation"
			KaelSys_ptr kptr={0x0008, 0x0000};

			//store pointer address
			KaelSys_ptr *topAddr = KaelSys_getAddr(kptr); 
			*topAddr=kptr;

			//store in register
			KaelSys_ptr *regAddr = KaelSys_getAddr(segTable[KS_REG]);
			regAddr+=arg1 * sizeof(KaelSys_ptr);
			*regAddr=kptr;
				
			return;
		case KS_STORE:
			uint8_t *regAddr2 = KaelSys_getAddr(segTable[KS_REG]);
			regAddr2+=arg1 * sizeof(KaelSys_ptr);
			*regAddr2=(uint8_t)arg2;
			return;
	}
	return;
}


void kaelSys_main(){
	//This function can only call instructions
	kaelSys_runInstr(KS_ALLOC, 1, 0);
	kaelSys_runInstr(KS_STORE, 3, 14);
	kaelSys_runInstr(KS_QUIT, 0, 0);
	return;
}


int main(){

	memRaw = calloc(1U<<20U, 1U);

	kaelSys_memRawInit(memRaw, headerData, headerSize, segTable, segTableSize);

	while(!quitFlag){
		kaelSys_main();
	}

	free(memRaw);

	return 0;
}
