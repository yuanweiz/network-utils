#include "Logging.h"
#include "Time.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
int main (){
    //close(1);
    int fd = 1;//=open("/dev/null", O_RDWR);
    if (fd!=1)return -1;
    Time start = Time::now();
    for (int i=0;i<1000000;++i){
        LOG_DEBUG << "";
    }
    close (fd);
    fprintf(stderr,"1000000 empty logs in %ld usecs\n", Time::now().since(start));
    return 0;
}
