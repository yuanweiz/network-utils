#ifndef __TIME_H
#define __TIME_H
#include <sys/types.h>
#include <string>
class Time{
public:
    Time()=delete;
    Time(const Time&)=default;
    static Time now();
    std::string toString();
    int64_t epochInUsec(){return epochInUsec_;}
    explicit Time(int64_t t):epochInUsec_(t){}
    int64_t since(Time start){return epochInUsec()-start.epochInUsec();}
    int64_t since(int64_t start){return epochInUsec()-start;}
    Time& add(int64_t);
private:
    int64_t epochInUsec_;
};

namespace detail{
    char* formatTime(Time);
}
#endif
