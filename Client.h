#ifndef __CLIENT_H
#define __CLIENT_H
#include <memory>
#include <vector>
#include "System.h"
class Channel;
class Eventloop;
class Client{
public:
    explicit Client(Eventloop* , Address*);
    ~Client();
private:
    std::unique_ptr<Channel> connnectChannel_;
    std::vector<std::unique_ptr<Channel>> conns_;
    void onChannelWritable();
    Eventloop * loop_;
};
#endif// __CLIENT_H

