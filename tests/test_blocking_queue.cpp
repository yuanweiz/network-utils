#include <assert.h>
#include <wz/Thread.h>
#include <wz/BlockingQueue.h>
#include <wz/CountDownLatch.h>
BlockingQueue<int> q;
CountDownLatch latch(10);

void producer(){
    for(int i=0;i<1000000;++i){
        q.put(i);
    }
    latch.countDown();
}

void consumer(){
    for(int i=0;i<1000000;++i){
        q.take();
    }
    latch.countDown();
}

int main(){
    for(int i=0;i<5;++i){
        Thread producerThread(producer, "producer");
        producerThread.start();
    }
    for(int i=0;i<5;++i){
        Thread consumerThread(consumer, "consumer");
        consumerThread.start();
    }
    latch.wait();
    assert(q.size()==0);
    return 0;
}
