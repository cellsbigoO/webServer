#include "ThreadPool.h"
#include <deque>
#include <unistd.h>

ThreadPool::ThreadPool(int threadnum) 
    : started_(false),
    threadnum_(threadnum),
    threadlist_(),
    taskqueue_(),
    mutex_(),
    condition_()
{

}


ThreadPool::~ThreadPool() {
    Stop();
    for(int i = 0; i < threadnum_; ++i) {
        threadlist_[i]->join();
    }
    for(int i = 0; i < threadnum_; ++i) {
        delete threadlist_[i];
    }
    threadlist_.clear();
}

void ThreadPool::Start() {
    if(threadnum_ > 0) {
        started_ = true;
        for(int i = 0; i < threadnum_; ++i) {
            std::thread *pthread = new std::thread(&ThreadPool::ThreadFunc, this);
            threadlist_.push_back(pthread);
        }
    }
}

void ThreadPool::Stop() {
    started_ = false;
    condition_.notify_all();
}

void ThreadPool::AddTask(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }
    condition_.notify_one();
}

void ThreadPool::ThreadFunc() {
    std::thread::id tid = std::this_thread::get_id();
    std::stringstream sin;
    Task task;
    while(started_) {
        std::cout << " run in: " << tid << std::endl;
        task = NULL;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(taskqueue_.empty() && started_) {
                condition_.wait(lock);
            }
            if(!started_) {
                break;
            }
            task = taskqueue_.front();
            taskqueue_.pop();
        }
        if(task) {
            try
            {
                task();
            }
            catch(std::bad_alloc& ba)
            {
                // bad_alloc caught in ThreadPool::ThreadFunc task
                ba.what();
                while(1);
            } 
        }
    }
}



// /***************************************测试***********************************/
// class Test{
// private:
// public:
//     void func(){
//         sleep(2);
//         std::cout << "Work in thread: " << std::this_thread::get_id() << std::endl;
//     }
// };
// int main()
// {

//     Test t;
//     ThreadPool tp(5);
//     tp.Start();
//     for (int i = 0; i < 1000; ++i)
//     {
//         // std::cout << "addtask" << std::this_thread::get_id() << std::endl;
//         tp.AddTask(std::bind(&Test::func, &t));
//     }
//     while(true);
//     return 0;
// }