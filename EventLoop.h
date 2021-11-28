#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H

#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "Poller.h"
#include "Channel.h"

class EventLoop{
public:
    typedef std::function<void()> Functor;
    typedef std::vector<Channel*> ChannelList;

    EventLoop();
    
    ~EventLoop();

    void loop();

    void AddChannelToPoller(Channel *pchannel) {
        poller_.AddChannel(pchannel);
    }

    void RemoveChannelToPoller(Channel *pchannel) {
        poller_.RemoveChannel(pchannel);
    }

    void UpdateChannelToPoller(Channel *pchannel) {
        poller_.UpdateChannel(pchannel);
    }

    void Quit() {
        quit_ = true;
    }

    std::thread::id GetThreadId() const {
        return tid_;
    }

    void WakeUp();

    void HandleError();

    void HandleRead();
    void AddTask(Functor functor) {
        {
            std::lock_guard <std::mutex> lock(mutex_);
            functorlist_.push_back(functor);
        }
        WakeUp();
    }

    void ExecuteTask() {
        std::vector<Functor> functorlist;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            functorlist.swap(functorlist_);
        }
        for(Functor &functor: functorlist) {
            functor();
        }
        functorlist.clear();
    }

private:
    std::vector<Functor> functorlist_;
    ChannelList channellist_;
    ChannelList activechannellist_;
    Poller poller_;
    bool quit_;
    std::thread::id tid_;
    std::mutex mutex_;
    int wakeupfd_;
    Channel wakeupchannel_;
};


#endif