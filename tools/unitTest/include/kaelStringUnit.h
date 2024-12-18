/** 
 * @file kaelStringUnit.h
 * 
 * @brief KaelStr unit test
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "kaelygon/global/kaelMacros.h"
#include "kaelygon/string/string.h"


void kaelString_unit(){
   KaelStr kstr = {0};
   KaelStr kstr2 = {0};
   kaelStr_alloc(&kstr2, 128);
   kaelStr_alloc(&kstr, 128);


   kaelStr_appendCstr(&kstr, "I am an appended string. ");
   kaelStr_pushCstr(&kstr, "This is appended on the left side.\n");
   kaelStr_print(&kstr);

   const char *exampleCstr = "This won't fit: \n1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
   kaelStr_pushCstr(&kstr, exampleCstr);

   kaelStr_resize(&kstr, 64);
   kaelStr_print(&kstr);
   kaelStr_setEnd(&kstr, 34);
   kaelStr_appendCstr(&kstr, "\nI was appended after truncation.\n");
   kaelStr_resize(&kstr, 128);

   char bufCstr[128]={0};
   kaelStr_toCstr(bufCstr, &kstr);
   printf("\nThat's a C string now -> {%s} <- EOF C string\n",bufCstr);
   printf("\n");

   kaelStr_fillCstr(&kstr, "I am a filler.\n");
   kaelStr_print(&kstr);
   printf("\n");

   kaelStr_setCstr(&kstr2, "I was a Cstr, now I am a KaelStr!\n");
   kaelStr_setKstr(&kstr, &kstr2);

   uint8_t isDifferentKstr = kaelStr_compareKstr(&kstr, &kstr2);
   uint8_t isDifferentCstr = kaelStr_compareCstr(&kstr, "I was a\n");
   printf("0 means equal.\n Compare: kstr and kstr2: %u\n Compare kstr and cstr: %u\n", isDifferentKstr, isDifferentCstr );

   char* kstrPtr  = kaelStr_getCharPtr(&kstr );
   char* kstrPtr2 = kaelStr_getCharPtr(&kstr2);
   printf("kstr  ptr: %p\n", (void *)kstrPtr );
   printf("kstr2 ptr: %p\n", (void *)kstrPtr2);

   printf("kstr size: %u\n", kaelStr_getSize(&kstr));
   printf("kstr2 end: %u\n", kaelStr_getEnd(&kstr2));

   printf("Inverted kstr: ");
   kaelStr_print(&kstr);

   kaelStr_appendKstr(&kstr,&kstr2);
   printf("kstr and kstr2 appended: ");
   kaelStr_reverseKstr(&kstr);

   kaelStr_print(&kstr);
   printf("\n");

   kaelStr_free(&kstr);
   kaelStr_free(&kstr2);


   printf("kaelString_unit done\n");
}