add_executable (kickoff kickoff.cpp) 
target_link_libraries (kickoff net)

add_executable (idleserver idleserver.cpp) 
target_link_libraries (idleserver net)

add_executable (idleclient idleclient.cpp) 
target_link_libraries (idleclient net)
