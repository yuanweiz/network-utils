#ifndef __EVENTLOOP_H
#define __EVENTLOOP_H
#include <vector>
#include <functional>
#include <poll.h>
//extern "C" struct pollfd;
class Channel;
class Eventloop{
public:
    using Functor = std::function<void()>;
    void loop();
    void callFunc(const Functor&);
    void add(Channel*);
    void unregister(Channel*);
    ~Eventloop();
private: 
    void fillPollfds();
    std::vector<Channel*>  channels_;
    
    //pollfds: used by fillPollfds() to reduce construction/destruction
    //of std::vector
    std::vector<struct pollfd> pollfds_;
    std::vector<Functor> funcs_;
};


#endif// __EVENTLOOP_H
