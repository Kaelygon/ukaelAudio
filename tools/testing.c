/**
 * @file kaelTerminalUnit.h
 * 
 * @brief Test string and terminal
 */

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



void kaelClockTesting() {

	kaelTui_rawmode(1); 

	KaelTui tui;
	kaelTui_alloc(&tui);

	KaelClock clock;
	kaelClock_init(&clock);

	KaelStr printBuffer;
	kaelStr_alloc(&printBuffer,128);
	kaelStr_setCstr(&printBuffer,"Hello world!");

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

		kaelTui_getKeyPressStr(&keyStr);
		
		if(kaelStr_getEnd(&keyStr)){
			if( kaelStr_compareCstr(&keyStr,KEY_SHIFT_Q)==0 ){kaelTui_setQuitFlag(&tui,1);}
			
			kaelStr_appendCstr(&charBuffer[0],"key{");
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

			char frameStr[12];
			sprintf(frameStr, "frame%u ", (uint16_t)kaelClock_getFrame(&clock)); 
			kaelStr_appendCstr(&charBuffer[0], frameStr);

         kaelStr_pushKstr(&printBuffer,&charBuffer[0]);

         kaelStr_clear(&charBuffer[0]);
			kaelStr_clear(&charBuffer[1]);
		}


		kaelTui_clearTerm();

      kaelStr_print(&printBuffer);

		fflush(stdout);

		kaelClock_sync(&clock); // sleep till next frame
	}
	uint64_t progEndTime=__rdtsc(); //debug 

	kaelStr_free(&printBuffer);
	kaelStr_free(&keyStr);
	for(uint8_t i=0;i<charBufCount;i++){
		kaelStr_free(&charBuffer[i]);
		charBufPtr[i]=NULL;
	}

	kaelTui_free(&tui);

	kaelTui_rawmode(0);

	printf("\n");
	printf("Time: %f\n",(double)(progEndTime-progStartTime)/CLOCK_SPEED_HZ); //debug
	printf("\n");
	

	printf("AUDIO_SAMPLE_RATE %u\n",AUDIO_SAMPLE_RATE);
	printf("CLOCK_SPEED_HZ %u\n",CLOCK_SPEED_HZ);
	printf("CLOCK_TARGET_RATIO %u\n",CLOCK_TARGET_RATIO);

	printf("kaelTerminal_unit Done\n");	
}


int main(){
   kaelClockTesting();
   return 0;
}