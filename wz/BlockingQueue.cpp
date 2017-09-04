#include <wz/BlockingQueue.h>
#include <assert.h>
template <class T>
BlockingQueue<T>::BlockingQueue()
    :mutex_(),cond_(&mutex_),queue_()
{
}

template <class T>
T BlockingQueue<T>::take(){
    Locker locker(&mutex_);
    while (queue_.empty()){
        cond_.wait();
    }
    assert(!queue_.empty());
    T ret=queue_.front();
    queue_.pop_front();
    return ret;
}

template <class T>
void BlockingQueue<T>::put(const T& item){
    Locker locker(&mutex_);
    queue_.push_back(item);
    cond_.signal();
}
