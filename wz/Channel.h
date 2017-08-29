#ifndef __CHANNEL_H
#define __CHANNEL_H
#include <functional>
#include "Context.h"
// Channel: wrapper class for posix pollfd/ epoll_event
// it introduces "another layer of indirection"
// this class doesn't own fd, Eventloop doesn't own Channel
class Eventloop;
class Channel : public EnableContext{
public:
    using Callback= std::function<void()>;
    Channel(Eventloop* loop, int fd);
    ~Channel();
    void enableRead();
    void disableRead();
    void enableWrite();
    void disableWrite();
    void disableAll();
    bool readable();
    bool writable();
    int events(){return events_;}
    void set_revents(int);
    void set_idx(int idx){idx_=idx;}
    void setReadCallback(const Callback&);
    void setWriteCallback(const Callback&);
    int idx(){return idx_;}
    void handleEvents();
    int fd();
    std::string reventString();
    std::string eventString();
private:
    void update();
    void handleRead();
    void handleWrite();
    int fd_;
    int idx_; //used by Eventloop::unregister()
    Eventloop * loop_;
    int events_;
    int revents_;
    Callback readCb_, writeCb_;
};

#endif// __CHANNEL_H
