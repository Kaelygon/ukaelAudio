/**
 * @file testUnits.c
 * 
 * @brief Run all unit tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "./include/kaelMemUnit.h"
#include "./include/kaelRandUnit.h"
#include "./include/kaelTerminalUnit.h"

int main(){
    kaelTerminal_unit();
    kaelMem_unit();
    kaelRand_unit();
    return 0;
}