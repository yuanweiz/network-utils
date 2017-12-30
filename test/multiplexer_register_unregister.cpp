#include "wz/Multiplexer.h"
#include "wz/Channel.h"
#include "wz/Thread.h"
#include <gtest/gtest.h>

struct MultiplexerRegisterUnregister: ::testing::Test{
    MultiplexerRegisterUnregister()
        :m(nullptr)
    {
    }
    Multiplexer m;
};

TEST_F( MultiplexerRegisterUnregister, Register){
}

int main(int argc,char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
