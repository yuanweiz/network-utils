add_executable (kickoff kickoff.cpp) 
target_link_libraries (kickoff wz)

add_executable (idleserver idleserver.cpp) 
target_link_libraries (idleserver wz)

add_executable (idleclient idleclient.cpp) 
target_link_libraries (idleclient wz)

add_executable (sinkclient sinkclient.cpp) 
target_link_libraries (sinkclient wz)

add_executable (chargenserver chargenserver.cpp) 
target_link_libraries (chargenserver wz)

add_executable ( logging_profile logging_profile.cpp)
target_link_libraries (logging_profile wz)

add_executable ( test_timer test_timer.cpp)
target_link_libraries (test_timer wz)

add_executable ( fs fs.cpp BlockManager.cpp BitmapView.cpp )
target_link_libraries (fs wz)
