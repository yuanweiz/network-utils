#include <unistd.h>
#include <wz/Thread.h>
#include <wz/CountDownLatch.h>
const int nWorkers=10;
CountDownLatch latch(nWorkers);
void worker(){
    usleep(5000);
    latch.countDown();
}
int main(){
    for(int i=0;i<nWorkers;++i){
        Thread thread(worker,"worker");
        thread.start();
    }
    latch.wait();
    return 0;
}
