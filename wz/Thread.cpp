#include "wz/Thread.h"
using namespace std;
struct Thread::ControlBlock{
    ControlBlock(const ThreadFunc&f):func(f)
    {
    }
    ControlBlock(ThreadFunc && f): func(std::move(f))
    {
    }
    std::function<void()> func; //make a copy
};

Thread::~Thread(){
    ::pthread_detach(pthread_);
}

void* Thread::threadFunc(void* ptr){
    auto * controlBlock = static_cast<Thread::ControlBlock*>(ptr);
    controlBlock->func();
    delete controlBlock;
    return nullptr;
}

Thread::Thread(const std::function<void()>& func)
    :func_(func)
{
}

void Thread::start(){
    //copy to heap
    auto * controlBlock = new ControlBlock(std::move(func_));
    ::pthread_create(&pthread_, nullptr, &Thread::threadFunc, controlBlock);
}

void Thread::join(){
    ::pthread_join(pthread_,nullptr);
}
