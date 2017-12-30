#include <wz/Eventloop.h>
#include <stdio.h>
#include <wz/Logging.h>
int main (){
    Eventloop e;
    int cnt=0;
    e.runEvery(0.2, [&cnt,&e](){
            LOG_DEBUG<<"hello";
            if (++cnt==3){
                e.quit();
            }
            });
    //e.runAfter(5.0, [&](){e.cancelTimer(id);});
    //e.runAfter(3.0, [&](){LOG_DEBUG<<"hello";e.quit();});
    e.loop();
    return 0;
}
