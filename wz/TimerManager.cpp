#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>
#include <algorithm>
#include "TimerManager.h"
using namespace std;

class Timer{
public:
    using Func=std::function<void()>;
    Timer(Time,const Func&,double,int64_t);
    Time time()const{return time_;}
    const Func& func(){return func_;}
    bool repeated() {return interval_>0;}
    int64_t uuid(){return uuid_;}
private:
    Time time_;
    Func func_;
    int64_t interval_ ;
    int64_t uuid_;
};

TimerManager::TimerManager(Eventloop* loop)
    :loop_(loop),
    fd_(timerfd_create(CLOCK_MONOTONIC,
            TFD_NONBLOCK | TFD_CLOEXEC)),
    lastUUID_(-1),
    channel_(new Channel(loop_,fd_)),
    handlingCallbacks_(false)
{
    channel_->enableRead();
    channel_->setReadCallback([this](){onReadable();});
}


void TimerManager::onReadable(){
    uint64_t ret;
    auto sz=::read(fd_,&ret,sizeof(ret));
    if (sz!=sizeof(ret)){
        //abort?
        abort();
    }
    //invariant
    assert( timers_.size() == validTimers_.size());

    Time now=Time::now();
    std::vector<Entry> expiredTimers;
    Entry entry = std::make_pair(now,nullptr); //Notice: which one is correct?
    {
    //std::pair<Time,Timer*> entry = std::make_pair(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it=timers_.lower_bound(entry);
    //make a copy, because func()() may remove timer from set, resulting in a dangle pointer access
    std::copy(timers_.begin(), it, std::back_inserter(expiredTimers));
    }
    timers_.erase(timers_.begin(), timers_.end());

    handlingCallbacks_ = true;
    for (auto & e: expiredTimers){
        e.second->func()();
    }
    handlingCallbacks_ = false;
    assert( timers_.size() == validTimers_.size());
}

TimerUUID TimerManager::add (Time time,const Func&func, double intervalSeconds){
    int64_t intervalInUsec = intervalSeconds * 1000000;
    if (intervalInUsec <= 0)
        intervalInUsec = -1;
    int64_t uuid=genUUID();
    Timer * timer = new Timer(time,func,intervalInUsec,uuid);
    //::timerfd_settime(fd_, 
    //FIXME: unique_ptr?
    timers_.insert({time,timer});
    assert(validTimers_.find(uuid)==validTimers_.end());
    validTimers_[uuid]=timer;
    return TimerUUID(uuid,timer);
}

void TimerManager::cancel (const TimerUUID& timerId){
    int64_t uuid = timerId.uuid_;
    auto it = validTimers_.find(uuid);
    if (it==validTimers_.end()){
        //already removed somewhere else
        return;
    }
    Timer* timer = it->second;
    assert(timer == timerId.timer_);
    Entry e = make_pair(timer->time(), timer);
    auto setIter = timers_.find(e);
    assert (setIter!=timers_.end());
    timers_.erase(setIter);
    assert( timers_.size() == validTimers_.size());
}

int64_t TimerManager::genUUID(){
    return ++ lastUUID_;
}
