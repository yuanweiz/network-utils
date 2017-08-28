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
class Eventloop{
public:
    using Functor = std::function<void()>;
    Eventloop();
    void loop();
    void quit();
    void callFunc(const Functor&);
    void add(Channel*);
    void unregister(Channel*);
    ~Eventloop();
    TimerUUID runAfter(double, const Functor&);
    TimerUUID runEvery(double, const Functor&);
    void cancelTimer(const TimerUUID&);
private: 
    bool polling_; //for debug use
    bool running_;
    void fillPollfds();
    std::vector<Channel*>  channels_;
    
    //pollfds: used by fillPollfds() to reduce construction/destruction
    //of std::vector
    std::vector<struct pollfd> pollfds_;
    std::vector<Functor> funcs_;
    std::unique_ptr<TimerManager> timerManager_;
};


#endif// __EVENTLOOP_H
