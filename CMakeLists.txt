project (wzlib CXX)

include_directories (${PROJECT_SOURCE_DIR})

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Werror -Wno-conversion -Wunused -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native" ) 
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_subdirectory ( examples)
add_subdirectory ( wz)
