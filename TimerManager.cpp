#include "TimerManager.h"
#include <iostream>
#include <thread>
#include <thread>
#include <ctime>
#include <ratio>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>


TimerManager* TimerManager::ptimermanager_ = nullptr;

std::mutex TimerManager::mutex_;
TimerManager::GC TimerManager::gc;
const int TimerManager::slotinterval_ = 1;
const int TimerManager::slotnum_ = 1024;

TimerManager::TimerManager() 
    : currentslot_(0),
    timewheel_(slotnum_, nullptr),
    runing_(false),
    th_()
{

}

TimerManager::~TimerManager() {
    Stop();
}

TimerManager* TimerManager::GetTimerManagerInstance() {
    if(ptimermanager_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_); // static的初始化是线程安全的
        if(ptimermanager_ == nullptr) {
            ptimermanager_ = new TimerManager();
        }
        return ptimermanager_;
    }
}

void TimerManager::AddTimer(Timer *ptimer) {
    if(ptimer == nullptr) 
        return ;
    std::lock_guard<std::mutex> lock(timewheelmutex_);
    CalculateTimer(ptimer);
    AddTimerToTimeWhell(ptimer);
}

void TimerManager::RemoveTimer(Timer* ptimer) {
    if(ptimer == nullptr) return ;
    std::lock_guard<std::mutex> lock(timewheelmutex_);
    RemoveTimerFromTimeWhell(ptimer);
} 

void TimerManager::AdjustTimer(Timer* ptimer) {
    if(ptimer == nullptr) return ;
    std::lock_guard<std::mutex> lock(timewheelmutex_);
    AdjustTimerToWheel(ptimer);
}

void TimerManager::CalculateTimer(Timer *ptimer) {
    if(ptimer == nullptr) return ;
    int tick = 0;
    int timeout = ptimer->timeout_;
    if(timeout < slotinterval_) {
        tick = 1;
    } else {
        tick = timeout / slotinterval_;
    }
    ptimer->rotation_ = tick / slotnum_;
    int timeslot = (currentslot_ + tick) % slotnum_;
    ptimer->timeslot_ = timeslot;
}

void TimerManager::AddTimerToTimeWhell(Timer* ptimer) {
    if(ptimer == nullptr) return ;
    int timeslot = ptimer->timeslot_;
    if(timewheel_[timeslot]) {
        ptimer->next_ = timewheel_[timeslot];
        timewheel_[timeslot]->prev_ = ptimer;
        timewheel_[timeslot] = ptimer;
    } else {
        timewheel_[timeslot] = ptimer;
    }
}

void TimerManager::RemoveTimerFromTimeWhell(Timer* ptimer) {
    if(ptimer == nullptr) 
        return ;
    int timeslot = ptimer->timeslot_;
    if(ptimer == timewheel_[timeslot]) {
        timewheel_[timeslot] = ptimer->next_;
        if(ptimer->next_ != nullptr) {
            ptimer->next_->prev_ = nullptr;
        }
        ptimer->prev_ = ptimer->next_ = nullptr;
    } else {
        if(ptimer->prev_ == nullptr) 
            return ;
        ptimer->prev_->next_ = ptimer->next_;
        if(ptimer->next_ != nullptr) 
            ptimer->next_->prev_ = ptimer->prev_;
        ptimer->prev_ = ptimer->next_ = nullptr;
    }
}

void TimerManager::AdjustTimerToWheel(Timer* ptimer) {
    if(ptimer == nullptr) 
        return ;
    RemoveTimerFromTimeWhell(ptimer);
    CalculateTimer(ptimer);
    AddTimerToTimeWhell(ptimer);
}

void TimerManager::CheckTimer() {
    std::lock_guard<std::mutex> lock(timewheelmutex_);
    Timer *ptimer = timewheel_[currentslot_];
    while(ptimer != nullptr) {
        if(ptimer->rotation_ > 0) {
            --ptimer->rotation_;
            ptimer = ptimer->next_;
        } else {
            ptimer->timercallback_();
            if(ptimer->timertype_ == Timer::TimerType::TIMER_ONEC) {
                Timer *ptemptimer = ptimer;
                ptimer = ptimer->next_;
                RemoveTimerFromTimeWhell(ptemptimer);
            } else {
                Timer *ptemptimer = ptimer;
                ptimer = ptimer->next_;
                AdjustTimerToWheel(ptemptimer);
                if(currentslot_ == ptemptimer->timeslot_ && ptemptimer->rotation_ > 0){
                    --ptemptimer->rotation_;
                }
            }
        }
    }
    currentslot_ = (++currentslot_) % TimerManager::slotnum_;
}
void TimerManager::CheckTick() {
    int si = TimerManager::slotinterval_;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int oldtime = (tv.tv_sec % 10000) * 1000 + tv.tv_usec / 1000; // 单位：毫秒
    int time;
    int tickcount;
    while(runing_) {
        gettimeofday(&tv, NULL);
        time = (tv.tv_sec % 10000) * 1000 + tv.tv_usec / 1000;
        tickcount = (time - oldtime) / slotinterval_;
        oldtime = oldtime + tickcount * slotinterval_;
        for(int i = 0; i < tickcount; ++i) {
            TimerManager::GetTimerManagerInstance()->CheckTimer();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500)); // 微秒
    }
}

void TimerManager::Start() {
    runing_ = true;
    th_ = std::thread(&TimerManager::CheckTick, this);
}

void TimerManager::Stop() {
    runing_ = false;
    if(th_.joinable()) {
        th_.join();
    }
}


/******************************************* 测试 *************************/
using namespace std::chrono;
class Test
{
public:
    
    steady_clock::time_point t1;

    Test(/* args */) : t1(steady_clock::now())
    {

    }
    ~Test()
    {

    }
    void func()
    {
        //std::cout << "timer test..."<< std::endl;        

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

        t1 = t2;
        std::cout << "It took me " << time_span.count() << " seconds." << std::endl;     
    }
};

// //timer unit test
// int main()
// {
//     Test test1;
//     Test test2;
    
//     TimerManager *ptimermanager = TimerManager::GetTimerManagerInstance();
//     //ptimermanager->AddTimer(1000, Timer::TimerType::TIMER_ONCE, std::bind(&Test::func, &test));
//     //ptimermanager->AddTimer(2000, Timer::TimerType::TIMER_PERIOD, std::bind(&Test::func, &test0));
//     //ptimermanager->AddTimer(1000, Timer::TimerType::TIMER_PERIOD, std::bind(&Test::func, &test2));
//     ptimermanager->Start();
//     // std::this_thread::sleep_for(std::chrono::seconds(2));

//     std::cout << "set timer "<< std::endl;      
//     Timer *timer1 = new Timer(100, Timer::TimerType::TimeR_PERIOD, std::bind(&Test::func, &test1));
//     // timer1->Start();

//     Timer *timer2 = new Timer(100, Timer::TimerType::TimeR_PERIOD, std::bind(&Test::func, &test2));
//     timer2->Start();
//     // timer1->Stop();

//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     // std::cout << "stop timer "<< std::endl; 
//     // Test test3;
//     // Timer *timer3 = new Timer(300, Timer::TimerType::TimeR_PERIOD, std::bind(&Test::func, &test3));
//     // timer3->Start();
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     // timer3->Adjust(500, Timer::TimerType::TIMER_ONEC, std::bind(&Test::func, &test3));
//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     delete timer1;
//     // delete timer2;
    
//     timer2->Stop();
//     ptimermanager->Stop();
//     return 0;
// }