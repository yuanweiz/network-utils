#include <wz/Multiplexer.h>
#include <wz/Logging.h>
#include <wz/Channel.h>
#include <assert.h>

Multiplexer::Multiplexer(Eventloop*loop)
    :loop_(loop)
{
    LOG_TRACE << "Multiplexer::Multiplexer()";
}
Multiplexer::~Multiplexer(){
}


void Multiplexer::update(Channel* ch){
    auto idx=ch->idx();
    assert (channels_[idx]==ch);
    pollfds_[idx].events = static_cast<short int>(ch->events());
}
void Multiplexer::poll(std::vector<Channel*>* _active){
    LOG_DEBUG << "Multiplexer::poll()";
    auto & active = * _active;
    //fillPollfds();
    assert(channels_.size() == pollfds_.size());
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
            num--;
            channels_[i]->set_revents(fd.revents);
            active.push_back(channels_[i]);
            //make a copy
            //Don't write code like this:
            //channels_[i]->handleEvents();
            //because handleEvents() may remove
            //elements from channels_
        }
    }
}
void Multiplexer::unregister(Channel* ch){
    int idx=ch->idx();
    assert (channels_[idx]== ch);
    std::swap( channels_[idx], channels_.back());
    channels_[idx]->set_idx(idx);
    channels_.pop_back();
    //channels_.back()=nullptr; //for debug
    
    //same for pollfds_
    std::swap( pollfds_[idx], pollfds_.back());
    pollfds_.pop_back();
    assert (pollfds_.size() == channels_.size());
}

void Multiplexer::fillPollfds(){
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

void Multiplexer::add(Channel* ch){
    assert(channels_.size() == pollfds_.size());
    ch->set_idx(static_cast<int>(channels_.size()));
    struct pollfd pfd;
    pfd.fd=ch->fd();
    pfd.events = ch->events();
    pfd.revents = 0;
    channels_.push_back(ch);
    pollfds_.push_back(pfd);
}
