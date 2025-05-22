#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "kaelygon/global/kaelMacros.h"

#define KAEL_BANK_BYTES (UINT16_MAX+1U)

typedef struct{
	const char *path;
	uint8_t **memBank;
	const uint16_t bankCount;
}KaelSys_disk;

void kaelSys_allocDisk(KaelSys_disk *disk, int mode){
	//Since this is just mimicing a 16-bit system, pointers are still 32-bit
	uint32_t flashBytes = KAEL_BANK_BYTES * disk->bankCount;

	//Open/create
	int32_t fileDesc = open(disk->path, mode, 0666);
	KAEL_ASSERT(fileDesc != -1);

	//Verify flash size
	KAEL_ASSERT(ftruncate(fileDesc, flashBytes) != -1);


	//Section map into banks
	disk->memBank = (uint8_t **)malloc(disk->bankCount*sizeof(uint8_t *));
	if(NULL_CHECK(disk->memBank)){
		return;
	}

	for(uint16_t i=0; i<disk->bankCount; i++){
		uint32_t bankOffset = i * KAEL_BANK_BYTES;
		void* tmpBank = mmap(NULL, KAEL_BANK_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc, bankOffset);
		KAEL_ASSERT(tmpBank != (void *)-1);
		disk->memBank[i] = (uint8_t *)tmpBank;
	}

	close(fileDesc);
}

void kaelSys_freeDisk(KaelSys_disk *disk){
	int32_t intCode;

	//Unmap
	for(uint16_t i=0; i<disk->bankCount; i++){
		void* flashPtr = (void*)(disk->memBank[i]);
		intCode=msync(flashPtr, KAEL_BANK_BYTES, MS_SYNC); 
		KAEL_ASSERT(intCode != -1);//assert triggered

		void* tmpBank = disk->memBank[i];
		intCode=munmap(tmpBank, KAEL_BANK_BYTES);
		KAEL_ASSERT(intCode != -1);

	}
	free(disk->memBank);
}


int main() {
	KaelSys_disk flash = {
		.path="./assets/flashDisk.bin",
		.memBank = NULL,
		.bankCount = 16,
	};
	KaelSys_disk rom = {
		.path="./assets/romDisk.bin",
		.memBank = NULL,
		.bankCount = 1,
	};
	
	//We keep rom writable till we have an actual rom 
	kaelSys_allocDisk( &rom, O_RDWR | O_CREAT );
	kaelSys_allocDisk( &flash, O_RDWR | O_CREAT );

	//Write flash
	for(uint16_t i=0; i<flash.bankCount; i++){
		char *bankPtr = (char *)flash.memBank[i];
		char string[20];
		sprintf(string,"Test bank %u.", i);
		strcpy(bankPtr, string);
		printf("Write: %s\n", bankPtr);
	}

	//Read flash
	for(uint16_t i=0; i<flash.bankCount; i++){
		char *bankPtr = (char *)flash.memBank[i];
		printf("Read: %s\n", (char*)bankPtr);
	}

	kaelSys_freeDisk(&rom);
	kaelSys_freeDisk(&flash);

	return 0;
}
