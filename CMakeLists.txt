# https://stackoverflow.com/questions/18968979/how-to-make-colorized-message-with-cmake
if(NOT WIN32) #Windows users don't deserve color
  string(ASCII 27 Esc)
  set(ColorReset "${Esc}[0m")
  set(Gray        "${Esc}[0;30m")
  set(Red         "${Esc}[0;31m")
  set(Green       "${Esc}[0;32m")
  set(Yellow      "${Esc}[0;33m")
  set(Blue        "${Esc}[0;34m")
  set(Magenta     "${Esc}[0;35m")
  set(Cyan        "${Esc}[0;36m")
  set(White       "${Esc}[0;37m")
  set(BGray       "${Esc}[1;30m")
  set(BRed        "${Esc}[1;31m")
  set(BGreen      "${Esc}[1;32m")
  set(BYellow     "${Esc}[1;33m")
  set(BBlue       "${Esc}[1;34m")
  set(BMagenta    "${Esc}[1;35m")
  set(BCyan       "${Esc}[1;36m")
  set(BWhite      "${Esc}[1;37m")
endif()


################# CMakefile #################
cmake_minimum_required(VERSION 3.20)
project(kaelAudio C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_COMPILER "gcc") 

set(DEFAULT_PROG 				"" ) # main.c base name
set(DEFAULT_BUILD_DIR 		"${CMAKE_SOURCE_DIR}/build" ) # Executables are built here
set(DEFAULT_SRC_DIR 			"${CMAKE_SOURCE_DIR}/src" ) # path to main.c
set(DEFAULT_INCLUDE_DIR 	"${CMAKE_SOURCE_DIR}/include" ) # path to header files root
set(DEFAULT_BUILD_TYPE		"RELEASE" ) # Options: RELEASE, DEBUG, ASAN
set(DEFAULT_BUILD_WHAT		"ALL" ) # Options: ACTIVE, ALL
set(DEFAULT_OMP_ENABLED 	"0" ) # Is omp multi-thread enabled? NOTE: Valgrind claims OMP is leaking ~300 bytes per thread

message("\n${BCyan}##########   CMake ${BWhite}start ${BMagenta}${CMAKE_PROJECT_NAME}${BCyan}   ##########\n")

################# Interpret parameters #################

#### Interpret directiories

set(BUILD_DIR ${DEFAULT_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

list(APPEND INCLUDE_DIR ${DEFAULT_INCLUDE_DIR}) #default include always included

if(NOT SRC_DIR) #Directory to the .c file with main()
	set(SRC_DIR ${DEFAULT_SRC_DIR})
	message("${Gray}Using default SRC_DIR ${SRC_DIR}")
endif()

### Interpret flags

if(NOT BUILD_WHAT) #ALL or ACTIVE
	set(BUILD_WHAT ${DEFAULT_BUILD_WHAT})
	message("${Gray}Using default BUILD_WHAT ${BUILD_WHAT}")
endif()

if(NOT BUILD_TYPE) #RELEASE DEBUG or ASAN
	set(BUILD_TYPE ${DEFAULT_BUILD_TYPE})
	message("${Gray}Using default BUILD_TYPE ${DEFAULT_BUILD_TYPE}")
endif()

#If no program specified, then build all 
if(NOT PROG) 
set(BUILD_WHAT "ALL") 
endif()

if(NOT OMP_ENABLED)
	set(OMP_ENABLED ${DEFAULT_OMP_ENABLED}) # OMP has some leak bug, so better disable it for malloc testing 
	message("${Gray}Using default OMP_ENABLED ${OMP_ENABLED}")
endif()

#capitalize so the input is case insensitive
if(BUILD_WHAT)
	string(TOUPPER ${BUILD_WHAT} BUILD_WHAT)
endif()
if(BUILD_TYPE)
	string(TOUPPER ${BUILD_TYPE} BUILD_TYPE)
endif()


# Set OMP
if(OMP_ENABLED)
	find_package(OpenMP REQUIRED)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
	set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fopenmp")
	set(FLAGS ${FLAGS} ${OpenMP_CXX_FLAGS})
endif()


### Find packages
find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)
pkg_check_modules(PKG_PipeWire REQUIRED IMPORTED_TARGET libpipewire-0.3)
set(LINK_LIBRARIES "${ALSA_LIBRARIES}" "${PIPESWIRE_LIBRARIES}" "PkgConfig::PKG_PipeWire")



###### Choose GCC flags

# Set optimization or debugger flags
set(CMAKE_BUILD_TYPE Debug)
set(DEFAULT_FLAGS "-g" "-Wall" "-Wextra" "-pedantic")

set(RELEASE_FLAGS "${DEFAULT_FLAGS}" "-Os" "-flto") # RELEASE
set(DEBUG_FLAGS "${DEFAULT_FLAGS}" "-D_GLIBCXX_DEBUG" "-O0") # DEBUG # "-fstack-usage" "-fno-omit-frame-pointer" # These two are commented due to code-oss midebugger bug
set(ASAN_FLAGS "${DEBUG_FLAGS}" "-fsanitize=undefined,address" "-fno-omit-frame-pointer") # ASAN same as debug flags


# Set flags by target
if(BUILD_TYPE STREQUAL "ASAN") 
	set(FLAGS "${ASAN_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

elseif(BUILD_TYPE STREQUAL "RELEASE") 
	set(FLAGS ${RELEASE_FLAGS})
	set(CMAKE_BUILD_TYPE ${BUILD_TYPE})

else() #Debug
	set(FLAGS ${DEBUG_FLAGS})
	set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
endif()





################# Functions #################
### ### ADD QUOTES TO ARGUMENTS THAT ARE 'LISTS' OR ONLY THE FIRST ELEMENT IS PASSED
### ### This was pain in the ass to debug. 


#Check if main file
function(f_is_main_file _file _result)
	file(STRINGS "${_file}" _file_contents REGEX "int[ \t]+main[ \t]*\\(.*\\)")
	if(_file_contents)
		set(${_result} TRUE PARENT_SCOPE)
	else()
		set(${_result} FALSE PARENT_SCOPE)
	endif()
endfunction()

#Remove or keep only the main() files in list of .c files
#_remMainFiles==1 removes main files, otherwise implementation files are removed
function(f_separate_main_files _sourceList _remMainFiles)
	set(_tempList "")  # Temporary list to hold files without main()

	foreach(_file ${${_sourceList}})
		f_is_main_file(${_file} _hasMain)  # Check if the file has a main() function

		if(_remMainFiles) #Delete main files
			if(NOT _hasMain) 
					list(APPEND _tempList ${_file})  
			endif()

		else() #Keep main files
			if(_hasMain) 
					list(APPEND _tempList ${_file})  
			endif()

		endif()
	endforeach()

	# Return the filtered list
	set(${_sourceList} ${_tempList} PARENT_SCOPE)
endfunction()

#Inject macros
function(f_inject_target_macro TARGET CONDITION MACRO)
	if(${CONDITION})
		target_compile_definitions(${TARGET} PRIVATE ${MACRO})
	endif()
endfunction()




###### Build functions

### BUILD_WHAT = ACTIVE
# Function to build a specific source file (active program)
function(f_build_active_source _mainDirectory _mainBaseName _implemList)

	#Verify it has main()
	set(_mainFile ${_mainDirectory}/${_mainBaseName}.c)
	if( NOT EXISTS ${_mainFile} )
		message("${Blue}${_mainFile} ${BRed}Doesn't exist. Are you trying to build a header file?")
		return()
	endif()
	f_is_main_file(${_mainFile} _isMainFile)
	if(NOT _isMainFile)
		message("${BRed}You are attempting to build implementation file: ${Blue}${_mainFile}")
		return()
	endif()


	#Set executable name suffix
	set(_progName ${_mainBaseName}_${BUILD_TYPE}) 
	#Link exectuable to the file with main()
	add_executable(${_progName} ${_mainFile}) 
	
	message("${Green}Building program ${Blue}${_progName}")


	#set build directory path
	set_target_properties(${_progName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
	#Set C standard
	target_compile_options(${_progName} PRIVATE $<$<CONFIG:${BUILD_TYPE}>:${FLAGS}> ) 
	#Link packages
	target_link_libraries(${_progName} PRIVATE ${LINK_LIBRARIES}) 
	### Link main() and implementation files
	set(_includeDir ${INCLUDE_DIR})


	#### Target specific includes
	set(_targetIncludeDir "${_mainDirectory}/include") 
	if(EXISTS ${_targetIncludeDir})
		message("${Gray}Target specific include found: ${Gray}${_targetIncludeDir}")
		list(APPEND _includeDir ${_targetIncludeDir})

		file(GLOB_RECURSE _targetImplemList ${_targetIncludeDir}/*.c) 
		list(APPEND _implemList ${_targetImplemList} )
	endif()


	###Add include folders for .h files
	target_include_directories(${_progName} PRIVATE ${_includeDir}) 
	### Link .c implementation files
	target_sources(${_progName} PRIVATE ${_implemList}) 


	#Inject debug macro
	f_inject_target_macro("${_progName}" "BUILD_TYPE STREQUAL DEBUG" "KAEL_DEBUG=1") 

endfunction()




### BUILD_WHAT = ALL helper
# Function to build all source files in directory
function(f_build_all_sources _includeSources)
	# Recursively find all .c main files from src directory
	file(GLOB_RECURSE _mainFileList ${SRC_DIR}/*.c) 
	f_separate_main_files(_mainFileList 0) # 0= keep only main files

	foreach(_mainFile ${_mainFileList}) #Build every single file that has main() in it
		get_filename_component(_mainBaseName ${_mainFile} NAME_WE) # File name with neither the directory nor the longest extension
		get_filename_component(_mainDirectory ${_mainFile} DIRECTORY)
		f_build_active_source("${_mainDirectory}" "${_mainBaseName}" "${_includeSources}")
	endforeach()
endfunction()




################# Build #################

# Create build directory
if(NOT (EXISTS ${BUILD_DIR}))
	file(MAKE_DIRECTORY ${BUILD_DIR})
endif()

# Recursively find implementation and header files
foreach(_includeDir ${INCLUDE_DIR})
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo_append "${Gray}Adding include folders: ") #print without new line
	if(EXISTS ${_includeDir})
		execute_process(COMMAND ${CMAKE_COMMAND} -E echo_append "${Gray}${_includeDir}" ) 

		file(GLOB_RECURSE _implemList ${_includeDir}/*.c) 
		list(APPEND IMPLEM_LIST ${_implemList} )
	endif()
	message("")
endforeach()

list (REMOVE_DUPLICATES IMPLEM_LIST)
f_separate_main_files(IMPLEM_LIST 1)  # 1 = remove only main files


###### Choose BUILD_WHAT

if(BUILD_WHAT STREQUAL "ACTIVE" AND PROG)
	f_build_active_source("${SRC_DIR}" "${PROG}" "${IMPLEM_LIST}")
	
elseif(BUILD_WHAT STREQUAL "ALL")
	message("${Green}TARGET ${BUILD_WHAT}")
	f_build_all_sources("${IMPLEM_LIST}")
	
endif()





################# make clean #################

### Functions

# makefile is too dumb to have loops so we generate 
# every file and remove command separately 
function(f_add_del_cmds _fileList _target)

	foreach(_file ${_fileList}) #iterate file list
		if(EXISTS ${_file})
			
			if(IS_DIRECTORY ${_file}) #add delete directory command
				add_custom_command(TARGET ${_target}
					POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E remove_directory ${_file}
					COMMENT "Removing file ${_file}"
					VERBATIM
				)
			else() #add delete file command
				add_custom_command(TARGET ${_target}
					POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E remove -f ${_file}
					COMMENT "Removing file ${_file}"
					VERBATIM
				)
			endif()

		endif()
	endforeach()
endfunction()

### Target clean_all

# List of files to be deleted using clean_all
list(APPEND _clean_file_list
	"${CMAKE_BINARY_DIR}/CMakeFiles"
	"${BUILD_DIR}"
	"${CMAKE_BINARY_DIR}/CMakeCache.txt"
	"${CMAKE_BINARY_DIR}/Makefile"
	"${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

add_custom_target(clean_all
   COMMAND ${CMAKE_BUILD_TOOL} clean
)
f_add_del_cmds("${_clean_file_list}" "clean_all") 

### End

message("")
message("${Green}Building: ${BWhite}${CMAKE_C_COMPILER} C${CMAKE_C_STANDARD} ${Gray}${SRC_DIR}/${PROG} ${FLAGS}")
message("${Green}Linker Libraries: ${Gray}${LINK_LIBRARIES}")
message("${Green}Build directory: ${Blue}${CMAKE_SOURCE_DIR}")


message("\n${BCyan}#########${BBlue}####   CMake${BMagenta} fini${BBlue}shed!   ####${BCyan}#########")
message("${ColorReset}")