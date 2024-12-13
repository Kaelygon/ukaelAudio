/**
 * @file kaelTerminalUnit.h
 * 
 * @brief Test string and terminal
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <x86intrin.h>
#include <signal.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/clock/clock.h" //cpu clock timer 
#include "kaelygon/terminal/terminal.h" //Text User Interface
#include "kaelygon/string/string.h" //KaelStr

#include "kaelygon/terminal/keyID.h" //KEY_ definitions as byte string



void kaelTerminal_unit() {

	kaelTui_rawmode(1); 

	KaelTui tui;
	kaelTui_alloc(&tui);

	KaelClock clock;
	kaelClock_init(&clock);

	KaelStr keyStr;
	kaelStr_alloc(&keyStr,8);

	uint8_t charBufCount=3;
	KaelStr charBuffer[charBufCount];
	char *charBufPtr[3];
	for(uint8_t i=0;i<charBufCount;i++){
		kaelStr_alloc(&charBuffer[i],32);
		charBufPtr[i]=kaelStr_getCharPtr(&charBuffer[i]);
	}

	uint64_t progStartTime=__rdtsc(); //debug to verify clock
	while (!kaelTui_getQuitFlag(&tui)) {
		if(kaelClock_getFrame(&clock) == 16){ //run for 16 frames
			kaelStr_appendCstr(&keyStr,(char*)KEY_SHIFT_Q); //Inject quit key
		}

		kaelTui_getKeyPressStr(&keyStr);
		
		if(kaelStr_getEnd(&keyStr)){
			if( kaelStr_compareCstr(&keyStr,KEY_SHIFT_Q)==0 ){kaelTui_setQuitFlag(&tui,1);}
			
			kaelStr_appendCstr(&charBuffer[0],"k{");
			for(uint16_t i=0;i<kaelStr_getEnd(&keyStr);i++){
				kaelStr_setCstr(&charBuffer[2],"%u,");
				if(i==kaelStr_getEnd(&keyStr)-1){
					kaelStr_setEnd(&charBuffer[2],2); //override ','
				}
				sprintf(charBufPtr[1], charBufPtr[2], (uint8_t)(kaelStr_getCharPtr(&keyStr)[i]));
				kaelStr_appendCstr(&charBuffer[0], charBufPtr[1]);
			}
			kaelStr_clear(&keyStr);
			kaelStr_appendCstr(&charBuffer[0],"} ");

			char frameStr[8];
			sprintf(frameStr, "f%u ", (uint16_t)kaelClock_getFrame(&clock)); 
			kaelStr_appendCstr(&charBuffer[0], frameStr);
			KAEL_ERROR_NOTE(kaelStr_getCharPtr(&charBuffer[0]));
			kaelStr_clear(&charBuffer[0]);
			kaelStr_clear(&charBuffer[1]);
		}

		kaelClock_sync(&clock); // sleep till next frame

		kaelTui_clearTerm();

		fflush(stdout);
	}
	uint64_t progEndTime=__rdtsc(); //debug 

	kaelStr_free(&keyStr);
	for(uint8_t i=0;i<charBufCount;i++){
		kaelStr_free(&charBuffer[i]);
		charBufPtr[i]=NULL;
	}

	kaelTui_free(&tui);

	kaelTui_rawmode(0);

	printf("\n");
	printf("Time: %f\n",(double)(progEndTime-progStartTime)/CLOCK_SPEED_HZ); //debug
	

	printf("kaelTerminal_unit Done\n");	
}

