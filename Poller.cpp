#include "Poller.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define EVENTNUM 4096
#define TIMEOUT 1000

Poller::Poller()
    : pollfd_(-1),
    eventlist_(EVENTNUM),
    channelmap_(),
    mutex_()
{
    pollfd_ = epoll_create(256);
    if(pollfd_ == -1) {
        perror("epoll_create1");
        exit(1);
    }
}

Poller::~Poller() {
    close(pollfd_);
}

void Poller::poll(ChannelList &activechannellist) {
    int timeout = TIMEOUT;
    int nfds = epoll_wait(pollfd_, &(*eventlist_.begin()), (int)eventlist_.size(), timeout);
    if(nfds == -1) {
        perror("epoll wait error");
    }
    for(int i = 0; i < nfds; ++i) {
        int events = eventlist_[i].events;
        Channel *pchannel = (Channel*)eventlist_[i].data.ptr;
        int fd = pchannel->GetFd();
        std::map<int, Channel*>::const_iterator iter;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            iter = channelmap_.find(fd);
        }
        if(iter != channelmap_.end()) {
            pchannel->SetEvents(events); // 由于 EPOLLONESHOT
            activechannellist.push_back(pchannel);
        } else {
            // not find channel !
        }
    }
    if(nfds == (int)eventlist_.capacity()) {
        eventlist_.resize(nfds * 2);
    }
}

void Poller::AddChannel(Channel *pchannel) {
    int fd = pchannel->GetFd();
    struct epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channelmap_[fd] = pchannel;
    }
    if(epoll_ctl(pollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        // epoll add error !
        exit(1);
    }
}

void Poller::RemoveChannel(Channel *pchannel) {
    int fd = pchannel->GetFd();
    struct  epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channelmap_.erase(fd);
    }
    if(epoll_ctl(pollfd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        // epoll del erro !
        exit(1);
    }
}

void Poller::UpdateChannel(Channel *pchannel) {
    int fd = pchannel->GetFd();
    struct  epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;
    if(epoll_ctl(pollfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        // epoll update error !
    }
}
