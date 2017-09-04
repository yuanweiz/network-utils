#ifndef __COUNTDOWN_LATCH_H
#define __COUNTDOWN_LATCH_H
#include <wz/Mutex.h>
#include <wz/Condition.h>
class CountDownLatch{
public:
    explicit CountDownLatch(int i);
    ~CountDownLatch();
    void wait();
    void countDown();
private:
    int count_;
    Mutex mutex_;
    Condition cond_;
};
#endif// __COUNTDOWN_LATCH_H
