#include <gtest/gtest.h>
#include <wz/Eventloop.h>
#include <wz/Thread.h>
#include <wz/Logging.h>


struct EventloopTest: ::testing::Test{
    Eventloop loop_;
};

TEST_F(EventloopTest, StartAndQuit){
    Thread quit([&] (){
            ::usleep(300*1000);
            loop_.quit();
            });
    quit.start();
    LOG_TRACE <<  "loop()";
    loop_.loop();
    quit.join();
}

int main(int argc,char**argv){
    Logger::setLevel(Logger::LogVerbose);
    LOG_TRACE <<  "logger";
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
