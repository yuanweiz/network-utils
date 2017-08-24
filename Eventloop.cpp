#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>
#include "Logging.h"
#include "Eventloop.h"
#include "Channel.h"

Eventloop::~Eventloop(){
}
void Eventloop::add(Channel*ch){
    channels_.push_back(ch);
}

void Eventloop::callFunc(const Functor& f){
    funcs_.push_back(f);
}

void Eventloop::loop(){
    while (true){
        fillPollfds();
        int num = ::poll(pollfds_.data(), pollfds_.size(), -1);
        if (num <0){
            //handle error;
            int err = errno;
            LOG_ERROR << err<<":"<<::strerror(err);
            abort();
        }
        for (size_t i = 0;i< pollfds_.size();++i){
            if (num==0)break;
            auto & fd = pollfds_[i];
            if (fd.revents!=0){
                channels_[i]->set_revents(fd.revents);
                channels_[i]->handleEvents();
            }
        }
        for(auto& f: funcs_){
            f();
        }
    }
}

void Eventloop::unregister(Channel* ch){
    int idx=ch->idx();
    assert (channels_[idx]== ch);
    std::swap( channels_[idx], channels_.back());
    channels_[idx]->set_idx(idx);
    channels_.pop_back();
}
void Eventloop::fillPollfds(){
    pollfds_.clear();
    //fixme: not so efficient
    for (auto ch: channels_){
        struct pollfd fd;
        fd.fd=ch->fd();
        fd.events=static_cast<short int>(ch->events());
        pollfds_.push_back(fd);
    }
}
