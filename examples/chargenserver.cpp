#include <sys/socket.h>
#include <unistd.h>
#include "wz/Server.h"
#include "wz/System.h"
#include "wz/Channel.h"
#include "wz/Logging.h"
#include "wz/Eventloop.h"
#include "wz/IgnSig.h"
#include "wz/Time.h"
#include <unordered_map>
#include <assert.h>

// example usage of Channel class
// the server holds an connection and do nothing,
// never proactively close the connection
class Channel;
//char buf[65536];

IgnSig ignSig;

//Logger::FinishFunc ;
std::vector<char> buf;
class MyServer{
public:
    explicit MyServer(Eventloop*);
private:
    void onNewConnection();
    void onPeerWritable(int);

    std::unique_ptr<Channel> channel_;
    Eventloop* loop_;
    std::unordered_map<int, std::unique_ptr<Channel>> peers_;
};

void func(LogStream&ls){
    static int i=0;
    i=(i+1)%100;
    if (i==0){
        ::write(1,ls.data(),ls.size());
    }
}

MyServer::MyServer(Eventloop * loop)
    :loop_(loop)
{
    const char* msg=NULL;
    buf.resize(4000*1000);
    do{
        //memset(buf,'a',sizeof(buf));
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

void MyServer::onNewConnection(){
    Address addr;
    int fd =System::accept(channel_->fd(), &addr);
    //store this connection in map
    auto pChan = new Channel(loop_,fd);
    LOG_DEBUG << "onNewConnection(): peer fd="<< fd
        << " at "<< addr.ipString() << " " << addr.port() 
        << " ,channel=" << pChan;
    pChan->enableWrite();
    pChan->setWriteCallback([this,fd](){onPeerWritable(fd);});
    pChan->ctx.i64[0]=0;
    pChan->ctx.i64[1]=Time::now().usecSinceEpoch();
    auto res = peers_.insert (std::make_pair(fd,std::unique_ptr<Channel>(pChan)));
    assert(res.second);
}

//static 
//LogStream& operator << (LogStream& ls,std::unordered_map<int,std::unique_ptr<Channel>>& mp ){
//    for (auto & p:mp){
//        ls<< p.first<<" ";
//    }
//    return ls;
//}

void MyServer::onPeerWritable(int fd){
    //LOG_TRACE << "onPeerWritable(), fd=" << fd <<" ";
    auto it = peers_.find(fd);
    assert(it!=peers_.end());
    auto & pChan=it->second;
    assert(pChan->fd()==fd);
    //try to write this socket
    auto & totalBytes = pChan->ctx.i64[0];
    auto & startTime = pChan->ctx.i64[1];
    int ret=write(fd,buf.data(),buf.size());
    //if ret >0 donothing
    if (ret<0){
        LOG_ERROR << ErrnoFmt(errno);
    }
    else {
        totalBytes += ret;
    }
    int64_t now=Time::now().usecSinceEpoch();
    double speed = 1000000.0 * totalBytes / (now - startTime);
    LOG_DEBUG << "speed: " << speed << "bytes/s";
}

int main (){
    //Logger::g_finish = func;
    Eventloop loop;
    MyServer server(&loop);
    loop.loop();
    return 0;
}
