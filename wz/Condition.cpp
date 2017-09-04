#include <wz/Mutex.h>
#include <wz/Condition.h>
void Condition::timedWait(double )
{
    //struct timespec ts;
    //auto sec = 
    //    ::pthread_cond_timedwait(&cond_,mutex_,nullptr);
}
Condition::Condition(Mutex* m)
    :mutex_(m->get())
{
    ::pthread_cond_init(&cond_,nullptr);
}
