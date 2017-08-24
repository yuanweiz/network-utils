#ifndef __LISTENER_H
#define __LISTENER_H
#include <memory>
#include <vector>
//#include "Channel.h"
#include "System.h"
class Eventloop;
class Channel;
class Listener{
public:
    Listener(Eventloop*);
private:
    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
};

struct Conn{
    int fd_;
    Address addr_;
};
class Server{
public:
    Server(Eventloop*);
private:
    void onNewConnection();
    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
    std::vector<Conn> peers_;
};
#endif// __LISTENER_H
