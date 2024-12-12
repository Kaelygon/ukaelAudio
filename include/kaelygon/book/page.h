//./include/kaelygon/book/page.h
// Sub item of book
// Manages shapes

#pragma once
/*

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kaelygon/string/string.h" //KaelStr
#include "kaelygon/math/math.h" //KaelStr
#include "kaelygon/kaelMacros.h"
#include "kaelygon/book/book.h"

#include "kaelygon/kaelMem.h"

struct KaelShape{
	KaelStr *kstr; //multibyte to fill the area or stored text
	uint16_t pos[4]; //{[x0,y0],[x1,y1]}
};

struct KaelPage{
	KaelMem *shape;
};

uint16_t kaelPage_empty(const KaelPage *page){
	if(NULL_CHECK(page)){return 1;}
	if(page->size==0){return 1;}
	return 0;
}

uint16_t kaelPage_size(const KaelPage *page){
	if(NULL_CHECK(page)){return 0;}
	return page->size;
}

//last shape
KaelShape *kaelPage_back(const KaelPage *page){
	if(NULL_CHECK(page) || kaelPage_empty(page)){return NULL;}
	if(NULL_CHECK(page->shape)){return NULL;};
	return &page->shape[page->size-1];
}

//shape by index
KaelShape *kaelPage_getShapePtr(const KaelPage *page, uint16_t shapeIndex){
	shapeIndex = kaelMath_min(shapeIndex , page->size-1); //at most last element
	if(NULL_CHECK(page) || kaelPage_empty(page)){return NULL;}
	if(NULL_CHECK(page->shape)){return NULL;};
	return &page->shape[shapeIndex];
}


void kaelPage_setShape(KaelShape *curShape, const KaelStr *inStr, const uint16_t inPos[]){
	//cap to last elements
	if( NULL_CHECK(curShape)){return;}
	if(NULL_CHECK(inStr) || NULL_CHECK(inPos) ) {KAEL_STORE_NOTE("NULLsetShapeArg"); return;}
	size_t strSize = kaelStr_getSize(inStr);
	kaelStr_realloc(curShape->kstr,strSize);
	kaelStr_setKstr(curShape->kstr,inStr);
	for (uint8_t i = 0; i < 4; i++) {
		curShape->pos[i]=inPos[i];
	}
}

void kaelPage_pushShape(KaelPage *page){
	if(NULL_CHECK(page)){return;}

	uint16_t newSize=page->size+1;
	KaelShape *newShapes = (KaelShape *)realloc(page->shape, newSize*sizeof(KaelShape));
	if(NULL_CHECK(newShapes,"push")){ return; }

	page->size =newSize;
	page->shape=newShapes;

	kaelPage_back(page)->kstr=kaelStr_alloc(0);
	
	for (uint8_t i = 0; i < 4; i++) {
		kaelPage_back(page)->pos[i]=0;
	}
}

void kaelPage_popShape(KaelPage *page){
	if(NULL_CHECK(page) || kaelPage_empty(page)){return;}

	kaelStr_free(&kaelPage_back(page)->kstr);

	uint16_t newSize=page->size-1;
	if(newSize==0){
		free(page->shape);
		page->shape=NULL;
	}else{
		KaelShape *newShapes = (KaelShape *)realloc(page->shape, newSize*sizeof(KaelShape));
		if( NULL_CHECK(newShapes,"pop") ){ return; }
		page->shape=newShapes;
	}

	page->size=newSize;
}

KaelMem* kaelPage_alloc(){
    KaelMem *page = kaelMem_new(sizeof(KaelPage));
	return page; 
}

void kaelPage_free(KaelMem *page){
	kaelMem_del(&page);
}

*/
