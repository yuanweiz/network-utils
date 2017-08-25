#include <memory>
#include <unordered_map>
#include "Logging.h"
#include "Eventloop.h"
#include "Channel.h"
#include "System.h"
#include <unistd.h>

using namespace std;

//initiate 1000 connections to the server
class IdleClient{
public:
    explicit IdleClient(Eventloop* loop)
        :loop_(loop),nConns_(0)
    {
        reconnect();
    }
private:
    void onNewConnection(){
        // here I simply discard the fds, DON'T DO THIS in real world
        LOG_TRACE << "IdleClient::onNewConnection()";
        //connnectChannel_.reset();
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);
        int fd=connnectChannel_->fd();
        int err= ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
        if (err){
            LOG_ERROR << "getsockopt(): " <<  ErrnoFmt(errno);
            reconnect();
            return;
        }
        if (optval){
            //FIXME: should retry at a time interval
            LOG_ERROR << "non-blocking connect() failed: " <<  ErrnoFmt(optval);
            ::close(connnectChannel_->fd());
            reconnect();
            return;
        }
        if (++nConns_ < 5) {
            LOG_DEBUG << "connection established, fd=" << fd;
            //FIXME: discard the fd, this causes resource leak.
            reconnect();
        }
        else{
            LOG_DEBUG << "release channel";
            connnectChannel_=nullptr;
        }
    }
    void reconnect(){
        Address host("127.0.0.1",9961);
        int conn = System::createNonBlocking();
        connnectChannel_.reset(new Channel(loop_, conn));
        connnectChannel_->enableWrite();
        connnectChannel_->setWriteCallback([this](){onNewConnection();});
        int err = ::connect( connnectChannel_->fd(),
                host.sockAddr(), host.sockSz());
        if (err>=0){
            LOG_ERROR << "nonblocking connect(), expect an -1 return value"
                "but got " << err;
        }
    }
    Eventloop * loop_;
    unique_ptr<Channel> connnectChannel_;
    int nConns_;
    //unordered_map<int, unique_ptr<Channel>> conns_;
};
int main (){
    Eventloop loop;
    IdleClient client(&loop);
    loop.loop();
}
