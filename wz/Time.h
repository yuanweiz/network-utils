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
    int64_t usecSinceEpoch(){return usecSinceEpoch_;}
    explicit Time(int64_t t):usecSinceEpoch_(t){}
    int64_t since(Time start){return usecSinceEpoch()-start.usecSinceEpoch();}
    int64_t since(int64_t start){return usecSinceEpoch()-start;}
    Time& add(int64_t);
    static Time fromNow(double sec);
    bool operator < ( Time rhs)const{return usecSinceEpoch_ < rhs.usecSinceEpoch_;}
    int64_t second()const;
    int64_t usecond()const;
    const static int64_t million = 1000000;
private:
    int64_t usecSinceEpoch_;
};

namespace detail{
    char* formatTime(Time);
}
#endif
