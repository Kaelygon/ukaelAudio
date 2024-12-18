
Restrictions
- Hypothetical 8-bit system limited to uint16_t and uint8_t types, no floating point or signed types. 
- If I don't lose my sanity before it, I'll make my_malloc. 
- Potentially simulate 16-bit address space with max 2mb rom. Would require memory banking. 
- Minimal file size and ram usage.
- I might adjust these restrictions on later time

Folder structure
```
/ukaelAudio
./.obsidian/        = Obsidian is stubborn to keep its configs here
./.vscode/          = Code OSS configuration that run bash script for each build type
./assets/           = Files that are read by a program go here
./build/            = CMake builds executables here
./CMakeFiles/       = CMake cache
./doxygen/          = Doxygen HTML doc
./generated/        = Program generated files go here
./include/kaelygon/ = Each related thing is on their own folders. kael- prefix is stripped out and related header (foo.h) and implementation (foo.c) files are in same folder
./src/              = Programs with main function
./tools/            = Testing programs and their headers in ./tools/*/include
```


Reason I use tabs is just a habit. I don't prefer spaces because they often aren't erased with 1 backspace and get unaligned if extra space ends up in code.
Tab = 3 spaces 
```C
	if(){
		while(){
			kaelFoo_stuffToDo();
		}
	}
```


Only types allowed are uint8_t and uint16_t
Few functions like KaelClock have to use 64-bit numbers to obtain ```__rdtsc()``` as I am still developing on 64-bit x86 system. These are just arbitrary limitations but it has been fun to work around problems with 16-bit uint arithmetic.
```C
//Round up *(3/8)
uint16_t number = 621
uint8_t threeEights = (uint16_t)(number*3 + 8/2)/8; //=233. Actual: 621*(3/8) = 232.875
```

For this reason some fundementals are different like _cmp functions. I decided to shift zero to 128 in all memcmp like functions for easier comparison
```C
	k32_cmp(a,b)
   if a==b, return 128
   if a>b, return 128+i
   if a<b, return 128-i
   where i is nth byte or bit depending on function
```

Default naming convention is camelCase.

```C  
KaelStr iAmAString;
kaelStr_alloc(&iAmAString,11);
```


Global variables, macros, enumerators are UPPER_SNAKE_CASE
Functions and variable names that are limited to the implementation file are marked with underscore ```_```
Arguments never have underscore and are in written camelCase
```C
#ifndef KAEL_DEBUG
	#define KAEL_DEBUG 1
#endif

typedef enum {
	KAEL_SUCCESS = 0,
}Kael_infoCode;

//global data, used only during debugging
extern KaelDebug *_GLOBAL_DEBUG;
```


Pseudo namespace
```C
typedef struct KaelFoo KaelFoo;
KaelFoo *kaelFoo_function(const char *someArgPtr);
```


Doxygen comments to generate doc. Comments in implementation files. Many functions are still missing these.
```C
/**
 * @file foo.c
 * 
 * @brief Example implementation file 
 * 
*/

#include "kaelygon/foo/fooHelper.h" //Implementation files can still have header files for related functionality that start taking too much space in implementation file

/**
 * @brief foo function example
 * 
 * @warning Important note
 * 
 * blah blah
 * 
 * @param stuff is some stuff
 * @param cstr
 * @return KAEL_SUCCESS, otherwise error 
*/
uint16_t kaelFoo_doStuff(void *stuff, char* cstr ){}

```


Global macro for NULL_CHECK(), this is to print out variable names instead of their addresses. 
In debug mode these output is stored in ```GLOBAL_DEBUG->infoStr[KAELDEBUG_DEBUG_STR]``` in kaelMacro.h
or index KAELDEBUG_NOTE_STR for notes
```C
	if (NULL_CHECK(foo)) { return; }
	if (NULL_CHECK(foo,"This is an additional note")) { return; }
	KAEL_ERROR_NOTE("kaelTree_get out of bounds"); 
```


Additionally there's assert for Debug build which is disabled in Release build.
NULL_CHECK is preferred for functions that aren't in tight loops. Ideally null check the variables before entering a tight loop as there are many things that still can go wrong.
Use KAEL_ASSERT during debug and mark clearly no NULL_CHECK is done in Release build.  
```C
/**
 * @brief base 256 rand
 * 
 * @warning No NULL_CHECK
 */
uint8_t k32_rand(kael32_t *seed){
	KAEL_ASSERT(seed!=NULL, "Arg in NULL");
	uint8_t carry = k32_u8mad(seed, seed, KAELRAND_MUL, KAELRAND_ADD);
	return seed->s[3] + carry;
}
```


If type is stored in heap, it has alloc and free functions
```C
void kaelFoo_alloc(KaelFoo *foo) {
	if (NULL_CHECK(foo)) { return; }
	foo->stuff = malloc(sizeof(KaelFoo));
}

void kaelFoo_free(KaelFoo *foo){
	if (NULL_CHECK(foo)) { return; }
	free(foo->stuff);
	*foo->stuff = NULL;
}
```


If type is exclusively stored in stack it may have init function
```C
void kaelFoo_init(KaelFoo *foo){
	if (NULL_CHECK(foo)) { return; }
	foo->stuff = 32768;
}
```


Use getter and setter functions for encapsulation. More often than not the values are modified before setting or return. 
```C
void kaelFoo_setStuff(uint16_t num){
	foo->stuff=num;	
}
uint16_t kaelFoo_getStuff(KaelFoo *foo){
	if (NULL_CHECK(foo)) { return; }
	return foo->stuff;	
}
```


Unit tests are located in ./tools/unitTest
```C
#include "kaelygon/foo/foo.h"

void kaelFoo_unit(){
	//Tests
}
```


Error codes are declared in global enumerator.
```C
typedef enum {
	 //General
	KAEL_SUCCESS			= 0,
	KAEL_ERR_NULL			= 128,

	 //KaelMem
	KAEL_ERR_FULL			= 131,

	 //KaelStr
	KAEL_ERR_MEM			= 130
}Kael_infoCode;

//128 to 255 errors, 127 to 1 warnings, 0=success
err = kaelStr_alloc(&str, 16)
if(!err		); // no error
if(err>127	); // error
if(err		);	// error or warning
if(err<128	);	// success or warning
```