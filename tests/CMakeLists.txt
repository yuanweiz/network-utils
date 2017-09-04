add_executable ( test_timer test_timer.cpp)
target_link_libraries (test_timer wz)

add_executable (test_thread test_thread.cpp) 
target_link_libraries (test_thread wz)

add_executable (test_mutex test_mutex.cpp) 
target_link_libraries (test_mutex wz)

add_executable (test_cond test_cond.cpp) 
target_link_libraries (test_cond wz)

add_executable (test_countdownlatch test_countdownlatch.cpp)
target_link_libraries (test_countdownlatch wz)

add_executable (test_blocking_queue test_blocking_queue.cpp)
target_link_libraries (test_blocking_queue wz)
