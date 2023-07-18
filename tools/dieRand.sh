#!/bin/bash
#
#ukaelReseed aims to have no repetition for at least 10 seconds
#or for 32768*10 samples of 8 bit samples. <1db variance
#ukaelRdtscSeed should be called much more rarely 
#so performance isn't as big of a deal.
#
#./tools/dieRand 2 | dieharder -B -g 200 -a

if [ -z "$1" ]
  then
    exit
fi

#tests favor ukaelRdtscSeed (TEST_NO 2) as this is more random than audacity white noise
#sh './tools/dieRand.sh' 1 | grep "FAIL\|WEAK"

TEST_NO=$1
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -t 32768 -p 200 -d 10
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -t 327680 -p 100 -d 1
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 12  -t 32768 -p 100
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 13  -t 32768 -p 100
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 15  -t 327680 -p 150
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 16  -t 131072 -p 100
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 101 -t 32768 -p 100 
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 102 -t 32768 -p 200 
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 203 -t 49152 -p 100
./tools/dieRand $TEST_NO | dieharder -W 0.01 -n 32 -B -g 200 -d 207 -t 131072 -p 100