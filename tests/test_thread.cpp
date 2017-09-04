#include <wz/Thread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <wz/Logging.h>
void testState(){
    int foo=42;
    struct dummy_s{} dummy_main;
    LOG_DEBUG << "from main thread: " << &dummy_main;
    auto hello = [&foo](){
        dummy_s dummy;
        LOG_DEBUG << "from hello thread: " << &dummy;
        foo=0;
        printf("hello\n");
    };
    Thread thread(hello,"hello");
    assert(!thread.started());
    assert(!thread.finished());
    thread.start();
    assert(thread.started());
    assert(!thread.finished());
    thread.join();
    assert(!thread.started());
    assert(thread.finished());
    assert(foo==0);
}

int main (){
    testState();
    return 0;
}
