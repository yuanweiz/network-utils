#ifndef __CONDITION_H
#define __CONDITION_H
#include <wz/Mutex.h>
#include <pthread.h>
class Mutex;
class Condition{
public:
    explicit Condition(Mutex* m);
    ~Condition(){
        ::pthread_cond_destroy(&cond_);
    }
    void wait(){
        ::pthread_cond_wait(&cond_,mutex_);
    }
    void signal(){
        ::pthread_cond_signal(&cond_);
    }
    void broadcast(){
        ::pthread_cond_broadcast(&cond_);
    }
    void timedWait(double time);
private:
    pthread_cond_t cond_;
    pthread_mutex_t * mutex_;
};
#endif// __CONDITION_H
