#include "Logging.h"
#include "Eventloop.h"
#include "Channel.h"
#include "Listener.h"
int main (){
    Eventloop e;
    //Channel ch (&e,0);
    //ch.enableRead();
    Server s(&e);
    e.loop();
    return 0;
}
