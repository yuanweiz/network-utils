#include <functional>
#include <memory>
#include <deque>
#include <unordered_map>
#include <set>
#include "Time.h"
#include "Channel.h"
#include "Eventloop.h"

class Timer; //opaque class

class TimerUUID {
    friend class TimerManager;
private:
    TimerUUID(int64_t uuid, Timer* timer):uuid_(uuid),timer_(timer){}
    int64_t uuid_;
    Timer * timer_; //redundant field, for debug use
};

class TimerManager{
public:
    using Func=std::function<void()>;
    explicit TimerManager(Eventloop*);
    TimerUUID add (Time ,const Func&, double intervalSeconds);
    void cancel (const TimerUUID &);
private:
    using Entry = std::pair<Time,Timer*>;
    void onReadable();
    int64_t genUUID();
    //std::deque<std::unique_ptr<Timer>> timers_;
    std::set< std::pair<Time,Timer*>> timers_;
    std::unordered_map<int64_t, Timer*> validTimers_;
    Eventloop* loop_;
    int fd_;
    int64_t lastUUID_;
    std::unique_ptr<Channel> channel_;
    //for debug use;
    bool handlingCallbacks_;
};
