#include "Time.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>


namespace detail{
static const int64_t million = 1000000;
__thread char timeStr[32];
__thread time_t cachedSecond=0;
__thread int secPartLen=-1;
char* formatTime(Time timeStamp){
    auto usecSinceEpoch_ = timeStamp.usecSinceEpoch();
    int64_t usec = usecSinceEpoch_ % million;
    int64_t sec_ = usecSinceEpoch_ / million;
    time_t sec = static_cast<time_t>(sec_);
    //copied from muduo/base/Timestamp.cc
    if (cachedSecond != sec){
        cachedSecond = sec;
        struct tm t;
        ::gmtime_r(&sec, &t);
        secPartLen=snprintf(timeStr, sizeof(timeStr), "%4d-%02d-%02d %02d:%02d:%02d",//.%06d",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                t.tm_hour, t.tm_min, t.tm_sec);
                //);
        int usecPartLen = snprintf (timeStr+ secPartLen, sizeof(timeStr)-secPartLen, ".%06d", static_cast<int>(usec));
        timeStr[secPartLen+usecPartLen]='\0';
    }
    else {
        snprintf (timeStr+ secPartLen, sizeof(timeStr)-secPartLen, ".%06d", static_cast<int>(usec));
    }
    return timeStr;
}
}

Time Time::now(){
    struct timeval tv;
    ::gettimeofday(&tv,0);
    int64_t usec=tv.tv_usec;
    int64_t sec=tv.tv_sec;
    return Time(sec * detail::million + usec );
}

Time Time::fromNow(double sec){
    int64_t lag = static_cast<int64_t>(sec * Time::million);
    return Time::now().add(lag);
}

Time& Time::add(int64_t diff){
    usecSinceEpoch_+=diff;
    return *this;
}

std::string Time::toString(){
    return std::string(detail::formatTime(*this));
}
