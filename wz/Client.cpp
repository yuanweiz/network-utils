#include "Client.h"
#include "Logging.h"
#include "Channel.h"
Client::Client(Eventloop*loop , Address* addr)
    :loop_(loop)
{
    int fd = System::createNonBlocking();
    int err = ::connect( fd, addr->sockAddr(), addr->sockSz());
    //expect err==-1
    if (err>=0){
        LOG_ERROR << "nonblocking connect(), expect an -1 return value"
            "but got " << err;
    }
    connnectChannel_.reset(new Channel(loop,fd));
    connnectChannel_->enableWrite();
    connnectChannel_->setWriteCallback([this](){onChannelWritable();});

}

void Client::onChannelWritable(){
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    int err= ::getsockopt(connnectChannel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen);
    if (err){
    }
    else {
        int fd=connnectChannel_->fd();
        connnectChannel_.release();
        auto conn = new Channel(loop_,fd);
        conn->enableRead();
        conn->enableWrite();
        conns_.emplace_back(conn);
    }
}
