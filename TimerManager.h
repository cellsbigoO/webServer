#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_

#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include "Timer.h"

class TimerManager {
public:
    typedef std::function<void()> CallBack;

    static TimerManager* GetTimerManagerInstance();

    void AddTimer(Timer* ptimer);

    void RemoveTimer(Timer* ptimer);

    void AdjustTimer(Timer* ptimer);

    void Start();

    void Stop();

    class GC {
    public:
        ~GC(){
            if(ptimermanager_ != nullptr) 
                delete ptimermanager_;
        }
    };

private:
    TimerManager();

    ~TimerManager();
    static TimerManager *ptimermanager_;
    static std::mutex mutex_;
    static GC gc;
    int currentslot_;
    static const int slotinterval_;
    static const int slotnum_;
    std::vector<Timer*> timewheel_;
    std::mutex timewheelmutex_;
    bool runing_;
    std::thread th_;

    void CheckTimer();
    void CheckTick();
    void CalculateTimer(Timer *ptimer);
    void AddTimerToTimeWhell(Timer *ptimer);
    void RemoveTimerFromTimeWhell(Timer *ptimer);
    void AdjustTimerToWheel(Timer* ptimer);
};

#endif