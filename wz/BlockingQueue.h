#ifndef __BLOCKING_QUEUE_H
#define __BLOCKING_QUEUE_H
#include <wz/Mutex.h>
#include <wz/Condition.h>
#include <deque>
template <class T>
class BlockingQueue{
public:
    using Queue = std::deque<T>;
    BlockingQueue()
        :mutex_(),cond_(&mutex_),queue_()
    {
    }
    T take(){
        Locker locker(&mutex_);
        while (queue_.empty()){
            cond_.wait();
        }
        assert(!queue_.empty());
        T ret=std::move(queue_.front());
        queue_.pop_front();
        return ret;
    }
    void put(const T& item){
        Locker locker(&mutex_);
        queue_.push_back(item);
        cond_.signal();
    }
    size_t size(){
        Locker locker(&mutex_);
        return queue_.size();
    }
private:
    Mutex mutex_;
    Condition cond_;
    Queue queue_;
};
#endif// __BLOCKING_QUEUE_H
