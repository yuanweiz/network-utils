#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <wz/Logging.h>
#include <wz/Eventloop.h>
#include <wz/Channel.h>
#include <wz/TimerManager.h>
#include <wz/Multiplexer.h>

Eventloop::Eventloop()
    : 
    multiplexer_(new Multiplexer(this)),
    timerManager_(new TimerManager(this))
{
    polling_=false;
    running_=true;
}

void Eventloop::quit(){
    running_=false;
}

TimerUUID Eventloop::runAfter(double interval, const Func & func){
    return timerManager_->add(Time::fromNow(interval),func,-1.0);
}
TimerUUID Eventloop::runEvery(double interval, const Func& func){
    return timerManager_->add(Time::fromNow(interval),func,interval);
}
void Eventloop::cancelTimer(const TimerUUID & uuid){
    timerManager_->cancel(uuid);
}

Eventloop::~Eventloop(){
}
void Eventloop::add(Channel*ch){
    multiplexer_->add(ch);
}

void Eventloop::callFunc(const Func& f){
    funcs_.push_back(f);
}

void Eventloop::updateChannel(Channel* ch){
    multiplexer_->update(ch);
}

void Eventloop::loop(){
    while (running_){
        std::vector< Channel*> active;
        polling_ = true;
        multiplexer_->poll(&active);
        polling_ = false;

        for ( auto * ch : active){
            ch->handleEvents();
        }
        for(auto& f: funcs_){
            f();
        }
    }
}

void Eventloop::unregister(Channel* ch){
    assert(!polling_);
    multiplexer_->unregister(ch);
}

