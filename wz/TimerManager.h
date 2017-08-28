#include <functional>
#include <memory>
#include <deque>
#include <unordered_map>
#include <set>
#include "Time.h"
#include "Channel.h"
#include "Eventloop.h"
#include <wz/TimerUUID.h>

class Timer; //opaque class
class TimerManager{
public:
    using Func=std::function<void()>;
    explicit TimerManager(Eventloop*);
    TimerUUID add (Time ,const Func&, double intervalSeconds);
    void cancel (const TimerUUID &);
private:
    using Entry = std::pair<Time,Timer*>;
    TimerUUID add (Timer*);
    void onReadable();
    bool dequeueExpiredTimers();
    void handleCallbacks();
    int64_t genUUID();
    //std::deque<std::unique_ptr<Timer>> timers_;
    std::set< std::pair<Time,Timer*>> timers_;
    std::unordered_map<int64_t, Timer*> validTimers_;
    Eventloop* loop_;
    int fd_;
    int64_t lastUUID_;
    std::unique_ptr<Channel> channel_;
    std::vector<Entry> expiredTimers_;
    //for debug use;
    bool handlingCallbacks_;
};
