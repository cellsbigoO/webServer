#include "Channel.h"
#include <iostream>
#include <sys/epoll.h>

Channel::Channel(): fd_(-1) {}

Channel::~Channel(){} 

void Channel::HandleEvent() {
    if(events_ & EPOLLRDBAND) { // 对方异常关闭事件
        closehandler_();
    } else if(events_ & (EPOLLIN | EPOLLPRI)) { // 对端有数据或者正常关闭
        readhandler_();
    } else if(events_ & EPOLLOUT) {
        writehandler_();
    } else {
        errorhandler_();
    }
}