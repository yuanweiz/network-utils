#ifndef __THREAD_H
#define __THREAD_H
#include <functional>
#include <pthread.h>
#include <string>
class Thread{
public:
    using Func=std::function<void()>;
    Thread(const Func&, const std::string&);
    ~Thread();
    void start();
    void join();
    bool finished()const;
    bool started()const;
    static pid_t currentThreadId();
    bool isCurrentThread() const;
private:
    static void* threadFunc(void *);
    struct ThreadData; //opaque fields, must be on heap
    ThreadData* data_; 
    enum {Ready,Starting, Running, Finished,Aborted/*really useful?*/}e_state_;
    //Func func_;
    pthread_t thread_;
};

void getThreadId();

#endif// __THREAD_H
