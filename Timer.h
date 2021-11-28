#ifndef _TIMER_H_
#define _TIMER_H_


#include <functional>

class Timer {
public:
    typedef std::function<void()> CallBack;

    typedef enum{
        TIMER_ONEC = 0,
        TimeR_PERIOD
    }TimerType;

    int timeout_; // 单位秒

    TimerType timertype_;

    CallBack timercallback_;

    int rotation_;

    int timeslot_;

    Timer *prev_;
    Timer *next_;

    Timer(int timeout, TimerType TimerType, const CallBack &timercallback);
    
    ~Timer();

    void Start();

    void Stop();

    void Adjust(int timeout, Timer::TimerType timertype, const CallBack &timercallback);
};


#endif