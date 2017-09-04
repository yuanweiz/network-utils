#include <wz/Thread.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>
using namespace std;
__thread pid_t t_tid=0;
namespace detail{
    pid_t gettid(){
        if (t_tid ==0){
            t_tid = syscall(SYS_gettid);
        }
        return t_tid;
    }
}
struct Thread::ThreadData{
    ThreadData(const Func& func, 
            const string& title)
        :func_(func), title_(title)
    {
        tid_=0;//for debug
    }
    bool isCurrentThread()const{
        return tid_== detail::gettid();
    }

    Func func_;
    pid_t tid_;
    string title_;
};

pid_t Thread::currentThreadId(){
    return detail::gettid();
}

Thread::Thread(const Func&func, const string& title)
    :data_(new ThreadData(func,title)),
    e_state_(Ready)
{
}

bool Thread::started()const{
    return e_state_ == Running;
}
bool Thread::finished()const{
    return e_state_ == Finished;
}

bool Thread::isCurrentThread()const{
    return data_->isCurrentThread();
}

Thread::~Thread(){
    delete data_;
}
void Thread::start(){
    //caution: cannot swap these two lines
    e_state_ = Starting;
    ::pthread_create(&thread_, NULL, &Thread::threadFunc, data_);
    e_state_ = Running;
}

void Thread::join(){
    void * ret;
    ::pthread_join(thread_ , &ret);
    e_state_ = Finished;
}

void* Thread::threadFunc(void *arg){
    auto* data= static_cast<Thread::ThreadData*>(arg);
    data->tid_ = detail::gettid();
    data->func_();
    return nullptr;
}

