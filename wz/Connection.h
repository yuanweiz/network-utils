#ifndef __CONNECTION_H
#define __CONNECTION_H
#include <functional>
#include <memory>
#include "Channel.h"
#include "Buffer.h"
class Eventloop;

class Connection{
public:
    using ReadCallback=std::function<void(Connection&,Buffer&)>;
    using WriteCompleteCallback=std::function<void(Connection&)>;
    using HighWaterMarkCallback=std::function<void(Connection&)>;

    //using WriteCallback=std::function<void(Connection&,Buffer&)>;
    Connection(Eventloop* loop, int fd);
    ~Connection();
    void enableRead(){channel_->enableRead();}
    bool readable(){return channel_->readable();}
    void disableRead(){channel_->disableRead();}
    bool writable() {return channel_->writable();}
    void setReadCallback(const ReadCallback&);
    void setWriteCallback(const WriteCompleteCallback&);
private:
    void handleRead();
    void handleWrite();
    void handleClose();
    Eventloop* loop_;
    Buffer recvBuffer_, sendBuffer_;
    
    ReadCallback readCb_;
    WriteCompleteCallback writeCompleteCb_;
    std::unique_ptr<Channel> channel_;
};
#endif// __CONNECTION_H
