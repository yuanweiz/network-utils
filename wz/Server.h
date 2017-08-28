#ifndef __LISTENER_H
#define __LISTENER_H
#include <memory>
#include <vector>
//#include "Channel.h"
#include "System.h"
class Eventloop;
class Channel;

class Server{
public:
    Server(Eventloop*);
    ~Server();
private:
    void onNewConnection();
    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
};
#endif// __LISTENER_H
