project (network CXX)
set (SRC 
    Logging.cpp Exception.cpp Buffer.cpp 
    Eventloop.cpp Channel.cpp Server.cpp 
    Time.cpp System.cpp Multiplexer.cpp
    TimerManager.cpp
    )

add_library(wz ${SRC})

#installation related
set (USER_VISIBLE_HEADERS Logging.h Exception.h Buffer.h Time.h StreamReader.h)
install (FILES ${USER_VISIBLE_HEADERS} DESTINATION /usr/include/wz )
install (TARGETS wz DESTINATION /lib )
