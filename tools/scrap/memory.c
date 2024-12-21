/**
 * @brief Interesting concepts, though likely rarely worth the complecity. 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
   
   const char* stanza[3] = {
   "    Never freed, I will leak.\n",
   "     Am I forever forsaken?\n",
   " Made to be forgotten is so rotten...\n\n"
   };

#define stackSize 16U
#define minSize sizeof(void *)
void **allocStack;
size_t freeTop=0;

typedef struct {
   int* index;
}Object;
void allocStack_init(){
   allocStack = calloc(stackSize, sizeof(void *));
   if(allocStack==NULL){return;}
   memset(allocStack,0,stackSize*sizeof(void *));
}

void *customMalloc(size_t size){
   size = size<minSize ? minSize : size ; //minimum size that few first byte can be safely compared
   if(freeTop>=(int)stackSize-1){
      printf("Full stack\n");
      return NULL;
   }
   void * ptr = calloc(size+1,1);
   if(!ptr){
      return NULL;
   }
   allocStack[freeTop++] = ptr;
   return ptr;
}

int custom_free(){
   if(allocStack==NULL){return 1;}
   do{
      free(allocStack[freeTop]);
      allocStack[freeTop]=NULL;
   }while(freeTop--);
   free(allocStack);
   allocStack=NULL;
   return 0;
}

Object *createObjects(int count){ //fill stack with varying sizes and types
   Object *obj = customMalloc(3*sizeof(Object *));
   for(int i=0; i<count; i++){
      obj[i].index = customMalloc(sizeof(int));
      if(obj[i].index==NULL){
         printf("Error\n");
         return NULL;
      }
      *obj[i].index=i;
   }
   return obj;
}

//addresses only stored in allocstack
char *loneFunction(){
   char* loneString = customMalloc(3*sizeof(char *)); 

   loneString = customMalloc( ( 3 + strlen(stanza[0]) + strlen(stanza[1]) + strlen(stanza[2]) ) * sizeof(char)  );
   if(!loneString){
      return NULL;
   }
   sprintf(loneString, "%s%s%s",stanza[0],stanza[1],stanza[2]);
   return loneString;
}

int searchLoneString(){
   int found=0;

   for(size_t i=1; i<freeTop; i++){
      if(allocStack[i]!=NULL){
         const char *compare = (char *)allocStack[i];
         const char *verse = stanza[0];
         size_t pos=0;
         while(pos<minSize && verse[pos]){
               if( compare[pos] != verse[pos] ){
                  break;
               }
               pos++;
         }
         if(pos>=8){
            found=1;
         }
      }
   }
   return found;
}

int main() {
   allocStack_init();

   Object *objList = NULL;
   objList = createObjects(3);

   char* loneString = loneFunction();
   if(loneString){
      printf(" LOG: Lone Object has been made.\n\n");
      printf(loneString);
   }

   loneString = NULL;

   int foundLoneString = searchLoneString();

   if(foundLoneString){
      printf("LOG: String match has been found.\n\n");
   }

   if(foundLoneString && objList){
      printf("   Worry not I hear your meek.\n");
      printf("   No exception will be raised.\n");
      printf("We'll be free at once without a leak!\n\n");
   }else{
      printf(" I wouldn't know if I will reset.\n");
      printf("  Keeping me you shall regret.\n"); allocStack = NULL;
      printf("  Nevertheless I won't forget.\n\n");
   }

   if(custom_free()==0){
      printf(" LOG: All memory has been freed.\n\n");
   }else{
      printf("ERR: memory has not been freed.\n\n");
   }

   return 0;
}
