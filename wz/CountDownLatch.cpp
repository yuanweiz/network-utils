#include <wz/CountDownLatch.h>
CountDownLatch::CountDownLatch(int i)
    :count_(i),mutex_(),cond_(&mutex_)
{
}

CountDownLatch::~CountDownLatch(){
}

void CountDownLatch::wait(){
    Locker locker(&mutex_);
    while(count_>0){
        cond_.wait();
    }
}

void CountDownLatch::countDown(){
    Locker locker(&mutex_);
    count_--;
    if(count_==0)
        cond_.broadcast();
}
