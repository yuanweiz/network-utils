#include <sys/socket.h>
#include "Listener.h"
#include "System.h"
#include "Channel.h"
#include <unistd.h>
#include "Logging.h"
//using Callback=Channel::Callback;
Listener::Listener(Eventloop * loop)
    :loop_(loop)
{
    int fd = System::createNonBlocking();
    channel_.reset(new Channel(loop ,fd));
    channel_->enableRead();
}

Server::Server(Eventloop * loop)
    :loop_(loop)
{
    const char* msg=NULL;
    do{
    int fd = System::createNonBlocking();
    System::setReuseAddr(fd);
    Address local("127.0.0.1",9961);
    int ret=bind(fd,local.sockAddr(),local.sockSz());
    if (ret<0) {msg="bind";break;}
    ret = listen(fd,5);
    if (ret<0) {msg="listen";break;}
    channel_.reset(new Channel(loop ,fd));
    channel_->enableRead();
    channel_->setReadCallback([this](){onNewConnection();});
    LOG_DEBUG << channel_->eventString();
    LOG_DEBUG << channel_->reventString();
    }while(false);
    if (msg)
        LOG_ERROR << msg << ErrnoFmt(errno);
    //channel_->setReadCallback();
}

void Server::onNewConnection(){
    Address addr;
    int fd =System::accept(channel_->fd(), &addr);
    LOG_DEBUG << "peer: fd="<< fd
        << addr.ipString() << addr.port();
    close(fd);
    //int fd=channel_->fd();
}
