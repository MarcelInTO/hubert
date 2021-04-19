# CMakeList.txt : CMake project for hubertTests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("hubertTests")

# Add source to this project's executable.
add_executable (hubertTests 
	"hubertTests.cpp" 
	"catch_amalgamated.cpp" 
	)

# TODO: Add tests and install targets if needed.

# These are necessary to compile hubert
target_compile_options(hubertTests PRIVATE "/Zc:__cplusplus")
target_compile_options(hubertTests PRIVATE "/std:c++latest")

#This is necessary to compile the amalgamated catch2 files
target_compile_options(hubertTests PRIVATE "/permissive")

# Add the hubert library include path
include_directories(../../include)

