#include "Timer.h"
#include <sys/time.h>
#include "TimerManager.h"

Timer::Timer(int timeout, TimerType timertype, const CallBack &timercallback) 
    : timeout_(timeout),
    timertype_(timertype),
    timercallback_(timercallback),
    rotation_(0),
    timeslot_(0),
    prev_(nullptr),
    next_(nullptr)
{
    if(timeout < 0) return ;
}

Timer::~Timer() {
    Stop();
}

void Timer::Start() {
    TimerManager::GetTimerManagerInstance()->AddTimer(this);
}
void Timer::Stop() {
    TimerManager::GetTimerManagerInstance()->RemoveTimer(this);
}

void Timer::Adjust(int timeout, Timer::TimerType timertype, const CallBack &timercallback) {
    timeout_ = timeout;
    timertype_ = timertype;
    timercallback_ = timercallback;
    TimerManager::GetTimerManagerInstance()->AdjustTimer(this);
}