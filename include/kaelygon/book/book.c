//./include/kaelygon/book/book.c
// Manages pages

#include <stdlib.h>
#include <stdint.h>

/*
#include "kaelygon/string/string.h" //KaelStr
#include "kaelygon/book/book.h"
#include "kaelygon/kaelMacros.h"
#include "kaelygon/kaelMem.h"

//#include "kaelygon/book/page.h" //constains very similar solution than book but manages shapes




uint16_t kaelBook_empty(const KaelBook *book){
	if(NULL_CHECK(book)){return 1;}
	if(book->size==0){return 1;}
	return 0;
}

uint16_t kaelBook_size(const KaelBook *book){
	if(NULL_CHECK(book)){return 0;}
	return book->size;
}

//last page
KaelPage *kaelBook_back(const KaelBook *book){
	if(NULL_CHECK(book) || book->size==0){return NULL;}
	if(NULL_CHECK(book->page)){return NULL;};
	return &book->page[book->size-1];
}

//page by index
KaelPage *kaelBook_getPagePtr(const KaelBook *book, uint16_t pageIndex){
	pageIndex = kaelMath_min(pageIndex , book->size-1); //at most last element
	if(NULL_CHECK(book)){return NULL;}
	if(NULL_CHECK(book->page)){return NULL;};
	return &book->page[pageIndex];
}

void kaelBook_pushPage(KaelBook *book){
	if(NULL_CHECK(book)){return;}

	uint16_t newSize=book->size+1;
	KaelPage *newPages = (KaelPage *)realloc(book->page, newSize*sizeof(KaelPage));
	if(NULL_CHECK(newPages,"push")){ return; }

	book->size=newSize;
	book->page=newPages;

	kaelBook_back(book)->shape=NULL;
	kaelBook_back(book)->size=0;
}

void kaelBook_popPage(KaelBook *book){
	if(NULL_CHECK(book) || book->size==0){return;}

	//free last page data
	kaelPage_free(kaelBook_back(book)); 

	//free the last page
	uint16_t newSize=book->size-1;
	if(newSize==0){ //free
		free(book->page);
		book->page=NULL;
	}else{ //resize
		KaelPage *newPages = (KaelPage *)realloc(book->page, newSize*sizeof(KaelPage));
		if( NULL_CHECK(newPages,"pop") ){ return; }
		book->page=newPages;
	}

	book->size=newSize;
}

KaelBook* kaelBook_alloc(){
	KaelBook *book = (KaelBook *)malloc(sizeof(KaelBook));
	if(NULL_CHECK(book)){return NULL;}

	book->page=NULL;
	book->index=0;
	book->size=0;

	return book; 
}

void kaelBook_free(KaelBook **book){
	if(NULL_CHECK(*book,"free")){return;}

	while(!kaelBook_empty(*book)){
		kaelBook_popPage(*book); //free last page
	}

	free(*book);
	*book=NULL;
	return;
}
*/
