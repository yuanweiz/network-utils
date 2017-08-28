add_executable (kickoff kickoff.cpp) 
target_link_libraries (kickoff net)

add_executable (idleserver idleserver.cpp) 
target_link_libraries (idleserver net)

add_executable (idleclient idleclient.cpp) 
target_link_libraries (idleclient net)

add_executable (sinkclient sinkclient.cpp) 
target_link_libraries (sinkclient net)

add_executable (chargenserver chargenserver.cpp) 
target_link_libraries (chargenserver net)

add_executable ( logging_profile logging_profile.cpp)
target_link_libraries (logging_profile net)
