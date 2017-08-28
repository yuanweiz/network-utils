#ifndef __IGNSIG_H
#define __IGNSIG_H
#include <signal.h>
class IgnSig{
    public:
    IgnSig(){
        ::signal(SIGPIPE, SIG_IGN);
    }
};
#endif// __IGNSIG_H
