#ifndef __SYSTEM_H
#define __SYSTEM_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
class Address;

namespace System{
int createNonBlocking();
int accept(int fd, Address*);
int setReuseAddr(int sockfd);
}

struct ErrnoFmt{
    explicit ErrnoFmt(int e):err(e){}
    int err;
};

class LogStream;
LogStream& operator << (LogStream& , ErrnoFmt );

class Address{
public:
    Address()=default;
    Address(const Address&)=default;
    Address(const std::string& ip, uint16_t port);
    struct sockaddr * sockAddr(){return reinterpret_cast<sockaddr*> (&addr);}
    static socklen_t sockSz(){return sizeof(struct sockaddr_in);}
    const char* ipString();
    uint16_t port(){return addr.sin_port;}
private:
    //char ipString_[16];
    struct sockaddr_in addr;
};
#endif// __SYSTEM_H
