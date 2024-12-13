//./include/kaelygon/terminal/terminal.h
//Text User Interface

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/terminal/keyMap.h"
#include "kaelygon/string/string.h"
#include "kaelygon/math/math.h"
#include "kaelygon/terminal/terminal.h"

#include "kaelygon/book/book.h"

uint8_t kaelTui_alloc(KaelTui *tui){
	if (NULL_CHECK(tui)) { return KAEL_ERR_ARG; }
	memset(tui,0,sizeof(KaelTui));

	tui->canvasSize[0]=120;
	tui->canvasSize[1]=36;
	tui->charSize=4; 

	tui->padding[0]=1; 
	tui->padding[1]=1;
	
	tui->viewSize[0]=tui->canvasSize[0]; 
	tui->viewSize[1]=tui->canvasSize[1];
	
	tui->debugLines=2;
	
	tui->virtualSize[0]=tui->canvasSize[0]+tui->padding[0];
	tui->virtualSize[1]=tui->canvasSize[1]+tui->padding[1];

	tui->rowBufSize = tui->virtualSize[0] * tui->charSize + tui->padding[0]; 
	uint8_t code = kaelStr_alloc( &tui->rowBuf, tui->rowBufSize ); 
	return code;
}

void kaelTui_free(KaelTui *tui){
	if (NULL_CHECK(tui)) { return; }
	kaelStr_free( &tui->rowBuf );
}

void kaelTui_setColor(int color_code) {
	printf("\033[38;5;%dm", color_code);
}

void kaelTui_clearTerm(){
	printf("\033[H\033[J");
} 

void kaelTui_signalQuit(int signal __attribute__((unused))) {
	kaelTui_clearTerm();
	fflush(stdout);
	kaelTui_rawmode(0);
	printf("Ctrl+C (SIGINT).\n");
	exit(0);
}

uint8_t kaelTui_getQuitFlag(const KaelTui *tui){
	return tui->quitFlag;
}

void kaelTui_setQuitFlag(KaelTui *tui, const uint8_t state){
	tui->quitFlag=state;
}

void kaelTui_getKeyPressStr(KaelStr *keyStr) {
	char *buf=kaelStr_getCharPtr(keyStr);
	size_t bytes = read(STDIN_FILENO, buf, kaelStr_getSize(keyStr));
	if(kaelMath_gtZeroSigned(bytes)){
		kaelStr_setEnd(keyStr,bytes);
	}
}

void kaelTui_rawmode(uint8_t state) {
	struct termios raw;
	tcgetattr(STDIN_FILENO, &raw);
	if(state){
		raw.c_lflag &= ~(ECHO | ICANON);
	}else{
		raw.c_lflag |= (ECHO | ICANON);
	}
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    int flags = fcntl(STDIN_FILENO, F_GETFL);
	if(state){
    	flags=flags | O_NONBLOCK;
	}else{
    	flags=flags & ~O_NONBLOCK;
	}
	fcntl(STDIN_FILENO, F_SETFL, flags);
}

void kaelTui_getViewSize(KaelTui *tui) {
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	tui->viewSize[0] = w.ws_col;
	tui->viewSize[1] = w.ws_row;
}



//write() canvas content and debug lines
void kaelTui_drawCanvas(KaelTui *tui) {
	return;
	kaelTui_getViewSize(tui);
	if(
		tui->viewSize[2]!=tui->viewSize[0] ||
		tui->viewSize[3]!=tui->viewSize[1]
	){ 
		tui->viewSize[2]=tui->viewSize[0];
		tui->viewSize[3]=tui->viewSize[1];
		tui->requestRefresh=1; 
	}
/*

	uint16_t startCol = tui->viewPos[0]+kaelMath_sub(tui->canvasSize[0], tui->viewSize[0]);
	uint16_t endCol = tui->viewSize[0]+tui->viewPos[0];
	endCol=kaelMath_min( endCol, tui->canvasSize[0] );

	uint16_t startRow = tui->viewPos[1]+kaelMath_sub(tui->canvasSize[1], tui->viewSize[1])+tui->debugLines;
	uint16_t endRow = startRow+tui->viewSize[1];
	endRow=kaelMath_min( endRow, tui->canvasSize[1] );
	kaelMath_sub( endRow, tui->debugLines );
*/
}
