#include <gtest/gtest.h>
#include <stdio.h>
#include "wz/Thread.h"
using ::testing::Test;

class ThreadLifetime: Test{
};

TEST(ThreadLifetime, MainThreadDiesFirst){
    auto func = [](){
        puts("thread started");
        ::sleep(1);
        puts("thread exited");
    };
    Thread thread (func);
    thread.start();
}
TEST(ThreadLifetime, Join){
    auto func = [](){
        puts("thread started");
        ::usleep(500*1000);
        puts("thread exited");
    };
    Thread thread (func);
    thread.start();
    thread.join();
}

TEST(ThreadLifetime, WorkerDiesFirst){
    auto func = [](){
        puts("thread started");
        puts("thread exited");
    };
    {
        Thread thread (func);
        thread.start();
        ::usleep(500*1000);
    }
}

int main (int argc, char** argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
