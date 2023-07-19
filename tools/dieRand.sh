#!/bin/bash
#
#ukaelReseed aims to have no repetition for at least 10 seconds
#or for 32768*10 samples of 8 bit samples. <1db frequency variance over 100 seconds
#
#ukaelTimeSeed should be called much more rarely, so it can be slower and more random
#ukaelTimeSeed is comparable to audacity rng used in white/brown noise
#tests favor ukaelTimeSeed (TEST_NO 2) as this is more random than audacity white noise
#81920 of 32bit samples = 327680 8bit samples
#
#./tools/dieRand 2 | dieharder -B -g 200 -a
#sh './tools/dieRand.sh' 1 | grep "FAIL\|WEAK\|PASS"

if [ -z "$1" ]
  then
    exit
fi

TEST_NO=$1
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 1
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 16384 -p 100 -d 2
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 12
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 15
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 16
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 100 
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 101
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 102 | grep "8|     8\|16|     8"
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 202 -n 4
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 203
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 204
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 205
./tools/dieRand $TEST_NO | dieharder -B -g 200 -t 81920 -p 100 -d 207