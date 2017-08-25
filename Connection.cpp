#include "Connection.h"
#include "Logging.h"
#include <unistd.h>
#include "System.h"

Connection::Connection(Eventloop* loop ,int fd)
    :loop_(loop),
    channel_(new Channel(loop,fd))
{
    channel_->setReadCallback([this](){handleRead();});
    channel_->setWriteCallback([this](){handleWrite();});
}

void Connection::handleRead(){
    char buf[65536];
    auto nRead=read(channel_->fd(),buf,sizeof(buf));
    if (nRead>0){
        if (nRead==sizeof(buf)){
            //TODO: do some logging?
        }
        recvBuffer_.append(buf,nRead);
        readCb_(*this,recvBuffer_);
    }
    else if (nRead == 0){
        handleClose();
    }
}
void Connection::handleClose(){
    channel_.release();
    //TODO: disconnectCb_();
}

void Connection::handleWrite(){
    if (sendBuffer_.readable()>0){
        //try send buffer first;
        auto nWrite = write(channel_->fd(),sendBuffer_.peek(), sendBuffer_.readable());
        if (nWrite <0){
            LOG_INFO
                << "Connection::handleWrite() returns -1, should be closed in next turn"
                << ErrnoFmt(errno);
        }
        else {
            sendBuffer_.retrieve(nWrite);
            if (0==sendBuffer_.readable())
                writeCompleteCb_(*this);
        }
    }
    //auto nWrite=::write(fd_
}

Connection::~Connection(){
    if (channel_)
        close(channel_->fd());
}
