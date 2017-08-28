#include <sys/socket.h>
#include <arpa/inet.h>
#include "System.h"
#include "Logging.h"
LogStream& operator << (LogStream& ls, ErrnoFmt e){
    ls << "errno=" << e.err << " (" << strerror(e.err) << ")";
    return ls;
}
namespace System{

//so-called "type rich programming"
struct ErrnoFmt{
    explicit ErrnoFmt(int e):err(e){}
    int err;
};


int setReuseAddr(int sockfd){
      int optval =  1 ;
      int ret= setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
              &optval, static_cast<socklen_t>(sizeof optval));
      return ret;
}
int createNonBlocking(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_ERROR << "createNonBlocking()";
    }
    return sockfd;
}

int accept(int fd, Address * addr){
    auto len = Address::sockSz();
    int ret=::accept(fd, addr->sockAddr(), &len);
    if (ret <0){
        LOG_ERROR << "System::accept";
    }
    return ret;
}

}

Address::Address(const std::string& ip, uint16_t port){
    ::bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_aton(ip.c_str(), &addr.sin_addr);
}

const char* Address::ipString(){
    return inet_ntoa(addr.sin_addr);
}
