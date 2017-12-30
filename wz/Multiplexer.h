#ifndef __MULTIPLEXER_H
#define __MULTIPLEXER_H
#include <vector>
#include <poll.h>
#include <functional>
class Eventloop;
class Channel;
class Multiplexer{
public:
    using Func=std::function<void()>;
    explicit Multiplexer (Eventloop*loop);//:loop_(loop){}
    ~Multiplexer();
    void poll(std::vector<Channel*> *);
    void add(Channel*);
    void unregister(Channel*);
    void update(Channel*);
    size_t size()const{return pollfds_.size();}
private:
    void fillPollfds();
    std::vector<struct pollfd> pollfds_;
    std::vector<Channel*>  channels_;
    Eventloop* loop_;
};
#endif// __MULTIPLEXER_H
