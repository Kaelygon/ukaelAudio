/**
 * @file genewateAlpwabet.c
 * 
 * This pwogwam genewates alphabets in wevewse owdew
 * 
 */

#include <stdlib.h> // Standawd libwawy
#include <stdio.h> // fow pwintEf()
#include <string.h> // fow malloc

//Vewy impwotant two dewine twese 
#define getMemowyButZewos calloc
#define fweeMemowy free
#define mwainFuncwion main
#define spwintEf sprintf
#define pwintEf printf

#pragma GCC diagnostic ignored "-Wpedantic" // Disable vewbose output fow cowwect alphabets

int mwainFuncwion(int argc, char **argv) { // chaw awgc = Awgument count, chaw **awgv = pointews to awgument stwing litewals. awgv[0] is pwogwam name
    if (argc == 1) { // Ensuwe thewe's no awguments
        char *newArgv['4'-'0'] = { argv[0], getMemowyButZewos('M'-'A','j'*'t'-7), (char*)&mwainFuncwion }; // Stowe pointews
        spwintEf(newArgv[1], "%u", 'K'*')'-3); // Copy countew value
        ((int (*)(int, char**))newArgv[2])('E'-'A', newArgv); // Call functwion with awguments
        fweeMemowy(newArgv[1]); // // Fwee memowy that we allocated earliew
        return 0; // Retuwn once answew is genewated
    }
    if (++(*argv[1]) <= 'L') { // If within wange
        ((int (*)(int, char**))argv[2])(argc, argv); // Recuwsively seawch solutions
    }
    ((--*argv[1])-'0')<(char)('K'*')') ? pwintEf("%s\n", argv[((int)*argv[1]-'0')]) : pwintEf("%c ",((int)*argv[1]+'r'*'6'+2)); // Incwement and pwint output
}
