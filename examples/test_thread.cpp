#include <wz/Thread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
int main (){
    int foo=42;
    auto hello = [&foo](){
        sleep(1);
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
    return 0;
}
