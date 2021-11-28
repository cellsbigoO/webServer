

#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include "EventLoop.h"

int CreateEventFd() {
    /*这里设置为FD_CLOEXEC表示当程序执行exec函数时本fd将被系统自动关闭,
    表示不传递给exec创建的新进程, 
    如果设置为fcntl(fd, F_SETFD, 0);
    那么本fd将保持打开状态复制到exec创建的新进程中*/
    int evfd = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(evfd < 0) {
        exit(1);
    }
    return evfd;
}

EventLoop::EventLoop()
    : functorlist_(),
    channellist_(),
    activechannellist_(),
    poller_(),
    quit_(true),
    tid_(std::this_thread::get_id()),
    mutex_(),
    wakeupfd_(CreateEventFd()),
    wakeupchannel_()
{
    wakeupchannel_.SetFd(wakeupfd_);
    wakeupchannel_.SetEvents(EPOLLIN | EPOLLET);
    wakeupchannel_.setReadHandle(std::bind(&EventLoop::HandleRead, this));
    wakeupchannel_.setErrorHandle(std::bind(&EventLoop::HandleError, this));
    AddChannelToPoller(&wakeupchannel_);
}

EventLoop::~EventLoop() {
    close(wakeupfd_);
}

void EventLoop::WakeUp() {
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_, (char*)(&one), sizeof one);
}

void EventLoop::HandleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof one);
}

void EventLoop::HandleError(){}

void EventLoop::loop() {
    quit_ = false;
    while (!quit_){
        poller_.poll(activechannellist_);
        for(Channel *pchannel: activechannellist_){
            pchannel->HandleEvent();
        }
        activechannellist_.clear();
        ExecuteTask();
    }
    
}