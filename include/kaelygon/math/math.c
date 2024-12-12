//./include/kaelygon/math/math.h
//Strictly unsigned arithmetic

#include <stdio.h>
#include <stdint.h>
#include "kaelygon/kaelMacros.h"

typedef uint16_t kmath_t;
static const kmath_t SIGN_MASK = 1U<<(sizeof(kmath_t)*CHAR_BIT-1);

#define KMATH_MAX UINT16_MAX

kmath_t kaelMath_min(kmath_t a, kmath_t b){
    return a<b ? a : b;
}

kmath_t kaelMath_max(kmath_t a, kmath_t b){
    return a>b ? a : b;
}

kmath_t kaelMath_isNegative(kmath_t a){
    return a&SIGN_MASK;
}

//Greater than 0 signed
kmath_t kaelMath_gtZeroSigned(kmath_t a){
    return (a!=0) && (!kaelMath_isNegative(a));
}

//ignore sign
kmath_t kaelMath_abs(kmath_t a){
    return a&~SIGN_MASK;
}

//uint subtract that doesn't underflow
kmath_t kaelMath_sub(kmath_t a, kmath_t b){
    return a >= b ? a - b : 0;
}

kmath_t kaelMath_rorr(kmath_t num, kmath_t shift){
    kmath_t invShift=sizeof(kmath_t)*CHAR_BIT;
    invShift=invShift-shift;
    num = (num>>shift) | (num<<invShift);
    return num;
}


uint8_t kaelMath_u8rorr(uint8_t num, uint8_t shift){
    uint8_t invShift=sizeof(uint8_t)*CHAR_BIT;
    invShift=invShift-shift;
    num = (num>>shift) | (num<<invShift);
    return num;
}

//other RLCG parameters
//{shift, mult, add} //Z score - period 65535
//{1, 95, 975}, // 0.316218
//{2, 83, 89},  // 0.374768
//{2, 61, 394}, // 0.513991
//{1, 49, 112}, // 0.645737
kmath_t kaelMath_lcg(kmath_t seed){
    seed = kaelMath_rorr(seed,2);
    seed = seed * 83 + 89;
    return seed;
}