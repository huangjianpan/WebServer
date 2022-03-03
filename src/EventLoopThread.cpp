#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread() : 
    loop_(nullptr), 
    thread_(&EventLoopThread::threadFunc, this) 
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!loop_) {
        cond_.wait(lock);
    }
}

void EventLoopThread::terminate() {
    if (thread_.joinable()) {
        loop_->quit();
        thread_.join();
    }
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }
    loop.loop();
}