#include <vector>
#include <assert.h>
#include <wz/Mutex.h>
#include <wz/Thread.h>
#include <wz/Logging.h>
Mutex mutex_;
int foo=0;
void threadFunc(){
    LOG_DEBUG << "Thread start";
    for (int i=0;i<100000;i++){
        Locker locker(&mutex_);
        ++foo;
    }
}
void testSum(){
    std::vector<Thread*> threads;
    LOG_DEBUG << "main thread";
    for (int i=0;i<10;++i){
        auto p = new Thread(threadFunc, "worker");
        threads.push_back(p);
        p->start();
    }
    for (auto *p:threads){
        p->join();
        delete p;
    }
    LOG_DEBUG << "done";
    assert(foo==1000000);
}

int main (){
    testSum();
    return 0;
}
