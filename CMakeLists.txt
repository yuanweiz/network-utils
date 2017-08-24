project (network CXX)
add_executable(main main.cpp Logging.cpp Exception.cpp Buffer.cpp Eventloop.cpp Channel.cpp Listener.cpp System.cpp)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wconversion -Wunused -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native" ) 
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

