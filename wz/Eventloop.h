#ifndef __EVENTLOOP_H
#define __EVENTLOOP_H
#include <vector>
#include <functional>
#include <poll.h>
#include <memory>
#include <wz/TimerUUID.h>
//extern "C" struct pollfd;
class Channel;
class TimerManager;
class Multiplexer;
class Eventloop{
public:
    using Func = std::function<void()>;
    Eventloop();
    void loop();
    void quit();
    void callFunc(const Func&);
    void add(Channel*);
    void unregister(Channel*);
    ~Eventloop();
    TimerUUID runAfter(double, const Func&);
    TimerUUID runEvery(double, const Func&);
    void cancelTimer(const TimerUUID&);
    void updateChannel(Channel*);
private: 
    bool polling_; //for debug use
    bool running_;
    
    //pollfds: used by fillPollfds() to reduce construction/destruction
    //of std::vector
    std::vector<Func> funcs_;
    std::unique_ptr<Multiplexer> multiplexer_;
    std::unique_ptr<TimerManager> timerManager_;
};


#endif// __EVENTLOOP_H
