#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <wz/Logging.h>
#include <wz/Eventloop.h>
#include <wz/Channel.h>
#include <wz/TimerManager.h>

Eventloop::Eventloop()
    : timerManager_(new TimerManager(this))
{
    polling_=false;
    running_=true;
}

void Eventloop::quit(){
    running_=false;
}

TimerUUID Eventloop::runAfter(double interval, const Functor & func){
    return timerManager_->add(Time::fromNow(interval),func,-1.0);
}
TimerUUID Eventloop::runEvery(double interval, const Functor& func){
    return timerManager_->add(Time::fromNow(interval),func,interval);
}
void Eventloop::cancelTimer(const TimerUUID & uuid){
    timerManager_->cancel(uuid);
}

Eventloop::~Eventloop(){
}
void Eventloop::add(Channel*ch){
    ch->set_idx(static_cast<int>(channels_.size()));
    channels_.push_back(ch);
}

void Eventloop::callFunc(const Functor& f){
    funcs_.push_back(f);
}

void Eventloop::loop(){
    while (running_){
        fillPollfds();
        assert(channels_.size() == pollfds_.size());
        int num = ::poll(pollfds_.data(), pollfds_.size(), -1);
        if (num <0){
            //handle error;
            int err = errno;
            LOG_ERROR << err<<":"<<::strerror(err);
            abort();
        }

        polling_ = true;
        std::vector< Channel*> active;
        for (size_t i = 0;i< pollfds_.size();++i){
            if (num==0)break;
            auto & fd = pollfds_[i];
            if (fd.revents!=0){
                num--;
                channels_[i]->set_revents(fd.revents);
                //make a copy
                active.push_back(channels_[i]);
                //Don't write code like this:
                // iterator invalidation!
                //
                //channels_[i]->handleEvents();
            }
        }
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
    int idx=ch->idx();
    assert (channels_[idx]== ch);
    std::swap( channels_[idx], channels_.back());
    assert(channels_.back()==ch);
    channels_[idx]->set_idx(idx);
    //channels_.back()=nullptr; //for debug
    channels_.pop_back();
}

void Eventloop::fillPollfds(){
    pollfds_.clear();
    assert(pollfds_.empty());
    //fixme: not so efficient
    for (auto ch: channels_){
        struct pollfd fd;
        fd.fd=ch->fd();
        fd.events=static_cast<short int>(ch->events());
        pollfds_.push_back(fd);
    }
}
