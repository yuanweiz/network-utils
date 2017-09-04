#include <assert.h>
#include <unistd.h>
#include <wz/Mutex.h>
#include <wz/Thread.h>
#include <wz/Condition.h>
#include <wz/Logging.h>
Mutex mut_countdown_;
Mutex mut_inc_;
int nWorkers=10;
Condition cond_(&mut_countdown_);
// maybe use CountDownLatch in the future
int foo=0;
void workerFunc(){
    for (int i=0; i< 100000;++i){
        Locker locker(&mut_inc_);
        ++foo;
    }
    Locker locker_countdown_(&mut_countdown_);
    --nWorkers;
    LOG_DEBUG << "Decreacing nWorkers to " << nWorkers;
    if (nWorkers==0){
        LOG_DEBUG << "Condition::signal()";
        cond_.signal();
    }
}

void testDetach(){
    LOG_DEBUG << "main thread";
    int expected=100000* nWorkers;
    for (int i=0;i<nWorkers;++i){
        Thread worker(workerFunc,"worker");
        worker.start();
        //detached
    }
    LOG_DEBUG << "waiting for cond";
    {
        Locker locker(&mut_countdown_);
        while(nWorkers>0){
            LOG_DEBUG<<"Condition::wait()";
            cond_.wait();
        }
    }
    LOG_DEBUG << "done";
    assert(foo==expected);
}

int main (){
    testDetach();
    return 0;
}
