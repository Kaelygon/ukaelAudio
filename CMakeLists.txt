# CMakefile

if(NOT OMP_ENABLED)
    set(OMP_ENABLED 1) # OMP has some leak bug, so better disable it for malloc testing 
endif()

cmake_minimum_required(VERSION 3.20)
project(kaelAudio C)

set(CMAKE_C_STANDARD 23)  # Set the C standard to C11

#Set directiories
if(NOT SRC_DIR) #program files that have main()
	set(SRC_DIR "./src")
endif()
if(NOT INCLUDE_DIR) #source and header files
	set(INCLUDE_DIR "./include")
endif()
set(BUILD_DIR "./build") #executables go here
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

include_directories(${INCLUDE_DIR})

string(TOUPPER ${BUILD} BUILD)
string(TOUPPER ${TYPE} TYPE)

if(NOT TYPE) 
	set(TYPE "DEBUG") #use debug flags by default
endif()

if(NOT PROG) 
	set(BUILD "ALL") #If no program specified build all 
endif()

# Find packages
find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)
find_package(Curses REQUIRED)
find_package(OpenMP REQUIRED)
pkg_check_modules(PKG_PipeWire REQUIRED IMPORTED_TARGET libpipewire-0.3)
set(LINK_LIBRARIES "${ALSA_LIBRARIES}" "${PIPESWIRE_LIBRARIES}" "PkgConfig::PKG_PipeWire" "m" "${CURSES_LIBRARIES}")

if(OMP_ENABLED)
    # Set OMP
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")
    set(FLAGS ${FLAGS} ${OpenMP_CXX_FLAGS})
endif()

#Set base variables
set(CMAKE_C_COMPILER "gcc" CACHE STRING "C Compiler" FORCE)

# Set optimization or debugger flags
set(CMAKE_BUILD_TYPE Debug)
set(RELEASE_FLAGS "-Os" "-flto" "-g" "-Wall"  "-Wextra" "-pedantic") # RELEASE
set(DEBUG_FLAGS "-g" "-Wall" "-Wextra" "-pedantic" "-D_GLIBCXX_DEBUG" "-fstack-usage" "-fno-omit-frame-pointer" "-O0") # DEBUG
set(ASAN_FLAGS "${DEBUG_FLAGS}" ) # ASAN same as debug flags


if(TYPE STREQUAL "ASAN") 
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(FLAGS ${ASAN_FLAGS})
elseif(TYPE STREQUAL "RELEASE") 
	set(FLAGS ${RELEASE_FLAGS})
    set(CMAKE_BUILD_TYPE ${TYPE})
else()
    set(FLAGS ${DEBUG_FLAGS})
    set(CMAKE_BUILD_TYPE ${TYPE})
endif()


#Inject macros
function(set_target_macros TARGET)
    if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
        target_compile_definitions(${TARGET} PRIVATE KAEL_DEBUG=1)
    else()
        target_compile_definitions(${TARGET} PRIVATE KAEL_DEBUG=0)
    endif()
endfunction()


# Function to build a specific source file (active program)
function(build_active_source PROG)
    add_executable(${PROG}_${TYPE} ${SRC_DIR}/${PROG}.c)
    set_target_properties(${PROG}_${TYPE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
    target_compile_options(${PROG}_${TYPE} PRIVATE $<$<CONFIG:${TYPE}>:${FLAGS}> ) # Set C standard
    file(GLOB_RECURSE INCLUDE_SOURCES ${INCLUDE_DIR}/*.c) # Recursively add .c files from include directory (except .h files)
    target_sources(${PROG}_${TYPE} PRIVATE ${INCLUDE_SOURCES})

    target_link_libraries(${PROG}_${TYPE} PRIVATE ${LINK_LIBRARIES})
    
    set_target_macros(${PROG}_${TYPE})
endfunction()


# Function to build all source files
function(build_all_sources)
    file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.c)
    foreach(source_file ${SOURCES})
        get_filename_component(target_name ${source_file} NAME_WE)
        add_executable(${target_name}_${TYPE} ${source_file})
        set_target_properties(${target_name}_${TYPE} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
        target_compile_options(${target_name}_${TYPE} PRIVATE $<$<CONFIG:${TYPE}>:${FLAGS}> )
        file(GLOB_RECURSE INCLUDE_SOURCES ${INCLUDE_DIR}/*.c) # Recursively add .c files from include directory (except .h files)
        target_sources(${target_name}_${TYPE} PRIVATE ${INCLUDE_SOURCES})

        target_link_libraries(${target_name}_${TYPE} PRIVATE ${LINK_LIBRARIES})

        target_link_libraries(${target_name}_${TYPE} PRIVATE ${LINK_LIBRARIES})
    endforeach()
endfunction()

if(BUILD STREQUAL "ACTIVE" AND PROG)
    message("Building ACTIVE")
    build_active_source(${PROG})
elseif(BUILD STREQUAL "ALL")
    message("Building ALL")
    build_all_sources()
endif()

message("Building C${CMAKE_C_STANDARD} ${SRC_DIR}/${PROG} using ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} ${FLAGS}")
message("Compiler Flags: ${CMAKE_C_FLAGS}")
message("Linker Libraries: ${LINK_LIBRARIES}")

#make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUILD_DIR})