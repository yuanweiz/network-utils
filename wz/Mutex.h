#ifndef __MUTEX_H
#define __MUTEX_H
#include <pthread.h>

class Mutex{
public:
    Mutex(){
        pthread_mutex_init(&mutex_,NULL);
        //TODO: error handling
    }
    ~Mutex(){
        pthread_mutex_destroy(&mutex_);
    }
    pthread_mutex_t * get(){return &mutex_;}
private:
    pthread_mutex_t mutex_;
};

class Locker{
public:
    explicit Locker (Mutex* mutex)
        :mutex_(mutex->get())
    {
        ::pthread_mutex_lock(mutex_);
    }
    void unlock(){
        ::pthread_mutex_unlock(mutex_);
        mutex_=nullptr;
    }
    ~Locker(){
        if (mutex_){
            ::pthread_mutex_unlock(mutex_);
        }
    }
private:
    pthread_mutex_t* mutex_;
};
#endif// __MUTEX_H
