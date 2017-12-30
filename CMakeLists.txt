cmake_minimum_required(VERSION 2.6)
project (wzlib CXX)

include_directories (${PROJECT_SOURCE_DIR})

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Werror -Wno-conversion -Wunused -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native" ) 
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_subdirectory ( wz)
if (BUILD_EXAMPLES)
add_subdirectory ( examples)
endif()

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

enable_testing()
find_package(GTest )
find_package(Threads)
if (GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_subdirectory(test)
endif()
