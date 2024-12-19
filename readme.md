
# ukaelAudio

Emulating system limited to uint8_t and uint16_t
Potentially further limit simulated addressable space to 16-bit bits and use memory banking.

At current stage the program doesn't have any functionality. I am still figuring out what tools I will need and developing terminal system as I go.

Goals:
- Run on as low end hardware as possibly. Mayhaps specs similar to Game Boy Color? 
- Under 8MB rom
- Audio: 
	 - Sample rate: 32768 hz
	 - 8 tracks
	 - Simple stereo by volume adjust, will see how viable other mixing is
	 - 16-bit Samples are sent to audio generator in 256 sample buffer
	 - While audio card is playing the buffer, a new buffer is being generated


Build all programs in ./src/   
Built binaries can be found in ./build/  
```C
cmake ./
make
```

Optional CMake configuration  
```C
cmake ./ -DBUILD_WHAT=${BUILD_WHAT} -DBUILD_TYPE=${BUILD_TYPE} -DSRC_DIR=${SRC_DIR} -DPROG=${PROG} -DOMP_ENABLED=${USE_OMP}"

Syntax -D[FLAG]=[VALUE]
```
```
-DPROG = Base name of main.c
-DSRC_DIR = Path to main.c
-DINCLUDE_DIR = Path to header files root
-DBUILD_TYPE = Options: RELEASE, DEBUG, ASAN
-DBUILD_WHAT = Options: ACTIVE, ALL
-DOMP_ENABLED = Is OMP multi-threading enabled? NOTE: Valgrind claims OMP is leaking few hundred bytes per thread. 
```
OMP is only used by testing tools. 

./vscode folder has [Code OSS ](https://github.com/microsoft/vscode)setup for all the build variants, it uses CMakeBuild.sh to simplify the arguments
Example of a generated command to build runUnitTests binary in ./tools/unitTest folder
```C
cmake ./CMakeLists.txt -DBUILD_WHAT=ACTIVE -DBUILD_TYPE=DEBUG -DSRC_DIR=./tools/unitTest -DPROG=runUnitTests -DOMP_ENABLED=0
```




