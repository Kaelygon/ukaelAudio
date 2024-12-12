//./include/kaelygon/math/rand.h
#pragma once
//8-bit permuted congruential generator rorr + LCG

//periods, total state bit count is KAELRAND_STATES*8 + 8
//32-bit period: 2^11 * 127^3
//24-bit period: 2^10 * 127^2 
//16-bit period: 2^9  * 127^1
//8-bit states 0, since iteration loop requires at least 1 state

//More shift,mul,add candidates at ./generated/validRorrLCG.txt

#include <stdint.h>
#include <stdlib.h>

enum {
	KAELRAND_SHIFT 		= 1,
	KAELRAND_MUL   		= 37,
	KAELRAND_ADD   		= 57,
	KAELRAND_STATES 	= 3
};

typedef struct {
	uint8_t state[KAELRAND_STATES]; 
	uint8_t last; //latest output, contributes to period length
}KaelRand;

void kaelRand_seed(KaelRand *krand, char *cstr);

uint8_t kaelRand(KaelRand *krand);

uint16_t kaelRand_u16lcg(uint16_t seed);

