#include "Channel.h"
#include "Eventloop.h"
#include "Logging.h"

Channel::Channel(Eventloop* loop, int fd)
    :fd_(fd),idx_(-1),loop_(loop),events_(0)
{
    loop_->add(this);
}

int Channel::fd(){return fd_;}
void Channel::set_revents(int ev){revents_=ev;}

void Channel::disableRead(){
    events_ &= (~POLLIN);
}
void Channel::disableAll(){
    events_ = 0;
}

void Channel::disableWrite(){
    events_ &= (~POLLOUT);
}

void Channel::enableRead(){
    events_ |= POLLIN;
}

void Channel::enableWrite(){
    events_ |= POLLOUT;
}

bool Channel::readable(){
    return 0!=(revents_ & POLLIN);
}

bool Channel::writable(){
    return 0!=(revents_ & POLLOUT);
}

void Channel::handleRead(){
    readCb_();
}

void Channel::handleWrite(){
    writeCb_();
}

void Channel::handleEvents(){
    if (readable()){
        handleRead();
    }
    if (writable()){
        handleWrite();
    }
}
void Channel::setReadCallback(const Callback& cb){ readCb_=cb;}
void Channel::setWriteCallback(const Callback& cb){ writeCb_=cb;}
static std::string eventString_(int events){
    std::string ret;
    if (events& POLLIN){
        ret+="|POLLIN";
    }
    if (events& POLLOUT){
        ret+="|POLLOUT";
    }
    return ret;
}
std::string Channel::eventString(){
    return ::eventString_(events_);
}
std::string Channel::reventString(){
    return ::eventString_(revents_);
}

Channel::~Channel(){
    if (loop_){
        loop_->unregister(this);
    }
}
