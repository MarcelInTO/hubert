# CMakeList.txt : CMake project for hubertTests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_VERBOSE_MAKEFILE on)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/out )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/out )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/out )

project ("hubertTests")

# Add source to this project's executable.
add_executable (hubertTests 
	"hubertTests.cpp" 
	"catch_amalgamated.cpp" 
	)


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # These are necessary to compile hubert
    target_compile_options(hubertTests PRIVATE "/Zc:__cplusplus")
    target_compile_options(hubertTests PRIVATE "/std:c++17")

    #This is necessary to compile the amalgamated catch2 files
    target_compile_options(hubertTests PRIVATE "/permissive")
else()
    target_compile_options(hubertTests PRIVATE "-std=c++17")
endif()

# Add the hubert library include path
include_directories(../../include)

