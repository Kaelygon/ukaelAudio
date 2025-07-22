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

#include "kaelygon/clock/clock.h" //cpu kaelClock timer 
#include "kaelygon/terminal/terminal.h" //Text User Interface
#include "kaelygon/string/string.h" //KaelStr

#include "kaelygon/terminal/keyID.h" //KEY_ definitions as byte string


#include <time.h>

//un-using namespace std;
clock_t std_clock() {
    return clock();
}

void kaelTerminal_unit() {

	kaelTui_rawmode(1); 

	KaelTui tui;
	kaelTui_alloc(&tui);

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

	KaelClock kaelClock;
	kaelClock_init(&kaelClock);

	double targetTimeRatio = 0.25; //debug to verify kaelClock //0.25 seconds
	uint16_t testTickCount = (uint16_t)(targetTimeRatio*((double)kaelClock.tickRate)); 
	clock_t progStartTime = std_clock(); 

	while (!kaelTui_getQuitFlag(&tui)) {
		if(kaelClock_getTick(&kaelClock) >= testTickCount){ 
			break;
		}

		kaelTui_getKeyPressStr(&keyStr);
		
		if(kaelStr_getEnd(&keyStr)){
			if( kaelStr_compareCstr(&keyStr,KEY_SHIFT_Q)==128 ){kaelTui_setQuitFlag(&tui,1);}
			
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

			char tickStr[12];
			sprintf(tickStr, "Tick %u:", (uint16_t)kaelClock_getTickHigh(&kaelClock)); 
			kaelStr_appendCstr(&charBuffer[0], tickStr);
			sprintf(tickStr, "%u ", (uint16_t)kaelClock_getTick(&kaelClock)); 
			kaelStr_appendCstr(&charBuffer[0], tickStr);

         kaelStr_pushKstr(&printBuffer,&charBuffer[0]);

         kaelStr_clear(&charBuffer[0]);
			kaelStr_clear(&charBuffer[1]);
		}


		kaelTui_clearTerm();

      kaelStr_print(&printBuffer);

		fflush(stdout);

		kaelClock_sync(&kaelClock); // sleep till next tick
	}
	clock_t progEndTime=std_clock(); //debug 

	kaelStr_free(&printBuffer);
	kaelStr_free(&keyStr);
	for(uint8_t i=0;i<charBufCount;i++){
		kaelStr_free(&charBuffer[i]);
		charBufPtr[i]=NULL;
	}

	kaelTui_free(&tui);

	kaelTui_rawmode(0);

	printf("\n");
	double resultTimeRatio = (double)(progEndTime-progStartTime)/CLOCKS_PER_SEC;
	printf("Time: %.8f\n",resultTimeRatio); //debug
	double deltaRatioAbs = targetTimeRatio > resultTimeRatio ? targetTimeRatio - resultTimeRatio : resultTimeRatio- targetTimeRatio;

	printf("%.4f sample discrepency. %.4f seconds\n",deltaRatioAbs*(double)AUDIO_SAMPLE_RATE, deltaRatioAbs*targetTimeRatio);
	if( deltaRatioAbs > 1.0/kaelClock.tickRate){ // +1 buffer lost
		printf("Did you set cpu kaelClock speed correctly when using __rdtsc?\n");
	}

	printf("\n");
	

	printf("AUDIO_SAMPLE_RATE %u\n",AUDIO_SAMPLE_RATE);
	printf("CLOCK_SPEED_HZ %u\n",CLOCK_SPEED_HZ);
	printf("CLOCK_TARGET_RATIO %u\n",CLOCK_TARGET_RATIO);

	printf("kaelTerminal_unit Done\n");	
}

