#include <sys/socket.h>
#include <unistd.h>
#include "Server.h"
#include "System.h"
#include "Channel.h"
#include "Logging.h"
#include "Eventloop.h"
#include <unordered_map>
#include <assert.h>

// example usage of Channel class
// the server holds an connection and do nothing,
// never proactively close the connection
class Channel;

class IdleServer{
public:
    explicit IdleServer(Eventloop*);
private:
    void onNewConnection();
    void peerConnection(int);

    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
    std::unordered_map<int, std::unique_ptr<Channel>> peers_;
};

IdleServer::IdleServer(Eventloop * loop)
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
}

void IdleServer::onNewConnection(){
    Address addr;
    int fd =System::accept(channel_->fd(), &addr);
    //store this connection in map
    auto pChan = new Channel(loop_,fd);
    LOG_DEBUG << "onNewConnection(): peer fd="<< fd
        << " at "<< addr.ipString() << " " << addr.port() 
        << " ,channel=" << pChan;
    pChan->enableRead();
    pChan->setReadCallback([this,fd](){peerConnection(fd);});
    auto res = peers_.insert (std::make_pair(fd,pChan));
    assert(res.second);
}

static 
LogStream& operator << (LogStream& ls,std::unordered_map<int,std::unique_ptr<Channel>>& mp ){
    for (auto & p:mp){
        ls<< p.first<<" ";
    }
    return ls;
}
void IdleServer::peerConnection(int fd){
    LOG_TRACE << "peerConnection(), fd=" << fd <<" ";
    auto it = peers_.find(fd);
    assert(it!=peers_.end());
    assert(it->second->fd()==fd);
    LOG_DEBUG << it->second->reventString();
    //try read this socket
    char buf[65536];
    int ret=read(fd,buf,sizeof(buf));
    if (ret==0){
        LOG_INFO << "closing socket "<< fd;
        LOG_DEBUG << peers_;
        peers_.erase(it);
        close(fd);
        //FIXME: how to RAII everything?
    }
    else if (ret<0){
        LOG_ERROR << "peerConnection(): "<< ErrnoFmt(errno);
    }
    else if (ret == sizeof(buf)){
        LOG_INFO << "potential high watermark issue?";
    }
}

int main (){
    Eventloop loop;
    IdleServer server(&loop);
    loop.loop();
    return 0;
}
