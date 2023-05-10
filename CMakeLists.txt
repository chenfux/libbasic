cmake_minimum_required(VERSION 3.5)

# -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-12
# set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")
# set(CMAKE_C_COMPILER "/usr/local/bin/gcc-12")

project(libbasic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)

enable_testing()

add_subdirectory(test)

# find a boost install with the libraries filesystem and system
find_package(Boost 1.81.0 REQUIRED)

# check if boost was found
if(Boost_FOUND)
    message ("boost found")
	include_directories(${Boost_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()
