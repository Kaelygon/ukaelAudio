#!/bin/bash
#This is intended for Code-OSS tasks.json
#CMakeBuild.sh [ALL,ACTIVE] [DEBUG, ASAN, RELEASE] [PATH TO main.c] [main.c BASENAME]
BUILD_WHAT="${1:-ALL}" #ALL: builds every .c source in SRC_DIR. ACTIVE: builds single source ${PROG}.c
BUILD_TYPE="${2:-RELEASE}" #Type of build: DEBUG, ASAN, RELEASE. GCC debug/optimization flags
SRC_DIR="${3:-./src}" #path to .c source. Uses ./include as .h directory
PROG="${4:-''}" #Base name of .c file if using ACTIVE.
USE_OMP="${5:-0}" 
CONFIG=./CMakeLists.txt

cmd="cmake ${CONFIG} -DBUILD_WHAT=${BUILD_WHAT} -DBUILD_TYPE=${BUILD_TYPE} -DSRC_DIR=${SRC_DIR} -DPROG=${PROG} -DOMP_ENABLED=${USE_OMP}"
echo "$cmd"
eval "$cmd"

make #VERBOSE=1
