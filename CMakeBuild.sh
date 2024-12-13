#!/bin/bash
#CMakeBuild.sh [ALL,ACTIVE] [DEBUG, ASAN, OPTIMIZED] [PATH TO .c] [.c BASENAME]
BUILD="${1:-ALL}" #ALL builds every .c source in SRC_DIR. ACTIVE builds single source ${PROG}.c
TYPE="${2:-OPTIMIZED}" #Type of build: DEBUG, ASAN, OPTIMIZED. GCC debug/optimization flags
SRC_DIR="${3:-./src}" #path to .c source. Uses ./include as .h directory
PROG="${4:-kaelAudio}" #Base name of .c file if using ACTIVE.
USE_OMP="${5:-1}" 
CONFIG=./CMakeLists.txt

cmd="cmake ${CONFIG} -DBUILD=${BUILD} -DTYPE=${TYPE} -DSRC_DIR=${SRC_DIR} -DPROG=${PROG} -DOMP_ENABLED=${USE_OMP}"
echo "$cmd"
eval "$cmd"

make #VERBOSE=1
