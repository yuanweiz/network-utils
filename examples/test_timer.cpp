#include <wz/Eventloop.h>
#include <stdio.h>
#include <wz/Logging.h>
int main (){
    Eventloop e;
    e.runEvery(1.0, [](){LOG_DEBUG<<"hello";});
    e.runEvery(1.5, [](){LOG_DEBUG<<"hello again";});
    //e.runAfter(3.0, [&](){LOG_DEBUG<<"hello";e.quit();});
    e.loop();
    return 0;
}
