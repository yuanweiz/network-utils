#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>
#include <algorithm>
#include <string.h>
#include "TimerManager.h"
#include <wz/Logging.h>
using namespace std;

class Timer{
public:
    using Func=std::function<void()>;
    explicit Timer(Time t,const Func&func,double interval,int64_t uuid)
        :time_(t),func_(func),interval_(interval*Time::million),uuid_(uuid)
    {
        if(interval_<=0) interval_=-1;
    }
    Time time()const{return time_;}
    const Func& func(){return func_;}
    bool repeated() {return interval_>0;}
    int64_t interval(){return interval_;}
    int64_t uuid(){return uuid_;}
private:
    Time time_;
    Func func_;
    int64_t interval_ ;
    int64_t uuid_;
};

void updateFd(int fd, Time time);
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

bool TimerManager::dequeueExpiredTimers(){
    expiredTimers_.clear();
    Time now=Time::now();
    Entry entry = std::make_pair(now,nullptr); //Notice: which one is correct?
    //LOG_DEBUG << "expiration="<< now.toString();
    //LOG_DEBUG << "expiredTimers.size()==" << expiredTimers_.size();
    //std::pair<Time,Timer*> entry = std::make_pair(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it=timers_.lower_bound(entry);
    if (it==timers_.begin()){
        return false;
    }
    if (it!=timers_.end()){
        //LOG_DEBUG << "next timer="<< it->second->time().toString();
    }
    //make a copy, because func()() may remove timer from set, resulting in a dangle pointer access
    std::copy(timers_.begin(), it, std::back_inserter(expiredTimers_));
    timers_.erase(timers_.begin(), it);
    assert(!expiredTimers_.empty());
    return true;
}

void TimerManager::handleCallbacks(){
    assert(tobeCancelled_.empty());
    handlingCallbacks_ = true;
    for (auto & e: expiredTimers_){
        e.second->func()();
    }
    handlingCallbacks_ = false;
}

// most complicated part in TimerManager,
// callback of a timer may remove 
// expired/unexpired timers, or even itself.
// if handlingCallbacks_ is true, 
// TimerManager::cancel() will remove 
// the timer, otherwise just append it to 
// toBeCancelled_ vector
void TimerManager::onReadable(){
    uint64_t ret;
    auto sz=::read(fd_,&ret,sizeof(ret));
    if (sz!=sizeof(ret)){
        //abort?
        abort();
    }
    //invariant
    assert( timers_.size() == validTimers_.size());
    dequeueExpiredTimers();
    if (expiredTimers_.empty()){
        //LOG_DEBUG << "warning: found no expired timers";
    }
    else {
        if (!timers_.empty()){
            updateFd(fd_, timers_.begin()->second->time());
        }
        else {
            LOG_TRACE << "all timers expired";
        }
    }
    tobeCancelled_.clear();
    handleCallbacks();

    //if repeated()
    std::vector<Timer*> repeatedTimers;
    for (auto &e:expiredTimers_){
        auto mapIt=validTimers_.find(e.second->uuid());
        assert(mapIt!= validTimers_.end());
        Timer * timer = mapIt->second;
        int64_t interval = timer->interval();
        Time nextTurn = timer->time().add(interval);
        if (timer ->repeated() && 
                tobeCancelled_.find(TimerUUID(timer->uuid(),timer))== tobeCancelled_.end()){
            //LOG_DEBUG <<"insert Timer "<< timer<<" back";
            repeatedTimers.push_back(
                    new Timer( nextTurn, timer->func(),
                        interval*1.0/Time::million, TimerManager::genUUID())
                    );
        }
        delete timer;
        validTimers_.erase(mapIt);
    }

    assert( timers_.size() == validTimers_.size());
    for (auto & id: tobeCancelled_){
        this->cancel(id);
    }
    //for repeated timers, insert it back
    for (Timer* timer: repeatedTimers){
        this->add(timer); //insert it back
    }
}

void updateFd(int fd, Time time){
    struct itimerspec spec;
    ::bzero(&spec,sizeof(spec));
    int64_t delay = time.usecSinceEpoch()- Time::now().usecSinceEpoch();
    spec.it_value.tv_sec = static_cast<time_t>(delay / Time::million);
    spec.it_value.tv_nsec = static_cast<long>(1000*(delay  % Time::million));
    if (spec.it_value.tv_nsec < 0 ) spec.it_value.tv_nsec = 100;
    //LOG_DEBUG << "spec: tv_sec="<< spec.it_value.tv_sec
    //    << ", tv_nsec=" << spec.it_value.tv_nsec;
    ::timerfd_settime(fd, 0, &spec, nullptr);
}

TimerUUID TimerManager::add (Time time,const Func&func, double intervalSeconds){
    int64_t uuid=genUUID();
    Timer * timer = new Timer(time,func,intervalSeconds,uuid);
    return this->add(timer);
}

TimerUUID TimerManager::add (Timer* timer){
    int64_t uuid = timer->uuid();
    Time time = timer->time();
    LOG_TRACE << "TimerManager::add( time="<<time.toString() 
        << ", interval=" << 1.0*timer->interval() /Time::million
        << "seconds )";
    bool needUpdateFd = false;
    auto it = timers_.begin();
    if ( it==timers_.end() ||  time < it->second->time() ){
        needUpdateFd = true;
    }
    if (needUpdateFd){
        updateFd(fd_, time);
    }
    //FIXME: unique_ptr?
    timers_.insert({time,timer});
    assert(validTimers_.find(uuid)==validTimers_.end());
    validTimers_[uuid]=timer;
    assert(timers_.size()== validTimers_.size());
    return TimerUUID(uuid,timer);
}

void TimerManager::cancel (const TimerUUID& timerId){
    if (!handlingCallbacks_){
        //LOG_DEBUG << "canceling when handlingCallbacks_==false";
        int64_t uuid = timerId.uuid_;
        assert( timers_.size() == validTimers_.size());
        auto it = validTimers_.find(uuid);
        if (it==validTimers_.end()){
            //LOG_DEBUG << "TimerUUID(Timer*="<< timerId.timer_
            //    << ", uuid="<<timerId.uuid_
            //    << ") already removed elsewhere";
            return;
        }
        validTimers_.erase(it);
        Timer* timer = it->second;
        assert(timer == timerId.timer_);
        Entry e = make_pair(timer->time(), timer);
        auto setIter = timers_.find(e);
        assert (setIter!=timers_.end());
        bool needUpdateFd=false;
        if (setIter == timers_.begin()){
            needUpdateFd=true;
        }
        setIter=timers_.erase(setIter);
        if (needUpdateFd && !validTimers_.empty()){
            updateFd(fd_,setIter->second->time());
        }
        assert( timers_.size() == validTimers_.size());
    }
    else {
        tobeCancelled_.insert(timerId);
        //LOG_DEBUG << "delay canceling procedure";
    }
}

int64_t TimerManager::genUUID(){
    return ++ lastUUID_;
}
