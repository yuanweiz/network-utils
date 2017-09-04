#include <wz/Thread.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string>
#include <assert.h>
#include <wz/Logging.h>
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
    assert(e_state_ == Running || finished());
    ::pthread_detach(thread_);
    LOG_DEBUG << "detached thread="<<thread_;
}

void Thread::start(){
    e_state_ = Starting;
    LOG_DEBUG << "starting";
    ::pthread_create(&thread_, NULL, &Thread::threadFunc, data_);
    LOG_DEBUG << "started";
    e_state_ = Running;
}

void Thread::join(){
    void * ret;
    ::pthread_join(thread_ , &ret);
    e_state_ = Finished;
}

void* Thread::threadFunc(void *arg){
    auto* data= static_cast<Thread::ThreadData*>(arg);
    //::prctl(PR_SET_NAME,data->title_.c_str());
    data->tid_ = detail::gettid();
    data->func_();
    delete data; //FIXME: is there a elegant way?
    return nullptr;
}

