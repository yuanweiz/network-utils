project (network CXX)
add_library(net 
    Logging.cpp Exception.cpp Buffer.cpp 
    Eventloop.cpp Channel.cpp Server.cpp System.cpp)

add_executable (main main.cpp)
target_link_libraries ( main net)

include_directories (${PROJECT_SOURCE_DIR})
add_subdirectory ( examples)

set (CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wconversion -Wunused -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native" ) 
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
