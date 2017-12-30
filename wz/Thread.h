#ifndef __THREAD_H
#define __THREAD_H
#include <pthread.h>
#include <functional>
#include <memory>
class Thread{
public:
    //we allow Thread object to be destructed (detached)
    //so we need to make a copy of thread local data
    //on the heap
    //
    //When constructed, no native thread is created,
    //when start()ed, all data are copied to heap,
    //func_ is also std::move()ed to new thread
    using ThreadFunc = std::function<void()>;
    void start();
    void join();
    Thread(const std::function<void()>& func);
    ~Thread(); //do pthread_detach() here
private:
    struct ControlBlock;
    bool started_;
    pthread_t pthread_;
    ThreadFunc func_;
    static void* threadFunc(void*);
    
};
#endif //__THREAD_H
