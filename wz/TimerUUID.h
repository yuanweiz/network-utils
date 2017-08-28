#ifndef __TIMERUUID_H
#define __TIMERUUID_H
#include <sys/types.h>
class Timer; //opaque class
class TimerUUID {
    friend class TimerManager; //opache class
private:
    TimerUUID(int64_t uuid, Timer* timer):uuid_(uuid),timer_(timer){}
    int64_t uuid_;
    Timer * timer_; //redundant field, for debug use
};
#endif// __TIMERUUID_H
