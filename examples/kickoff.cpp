#include <sys/socket.h>
#include <unistd.h>
#include "Server.h"
#include "System.h"
#include "Channel.h"
#include "Logging.h"
#include "Eventloop.h"

// example usage of Channel class
class Channel;

class KickoffServer{
public:
    explicit KickoffServer(Eventloop*);
private:
    void onNewConnection();
    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
};

KickoffServer::KickoffServer(Eventloop * loop)
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
    }while(false);
    if (msg)
        LOG_ERROR << msg << ErrnoFmt(errno);
    //channel_->setReadCallback();
}

void KickoffServer::onNewConnection(){
    Address addr;
    int fd =System::accept(channel_->fd(), &addr);
    LOG_DEBUG << "peer: fd="<< fd
        << " at "<< addr.ipString() << " " << addr.port();
    close(fd); //kick it off
    //int fd=channel_->fd();
}

int main (){
    Eventloop loop;
    KickoffServer server(&loop);
    loop.loop();
    return 0;
}
