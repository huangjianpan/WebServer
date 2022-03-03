#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool() :
    start_(false),
    baseLoop_(nullptr),
    nextId_(0)
{}

void EventLoopThreadPool::start(std::size_t numThreads) {
    if (!start_) {
        numThreads_ = numThreads;
        threads_.reserve(numThreads_);
        for (std::size_t i = 0; i < numThreads_; ++i) {
            threads_.emplace_back(std::make_unique<EventLoopThread>());
        }
        start_ = true;
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    return numThreads_ == 0 ? 
           baseLoop_ : 
           threads_[(nextId_ == numThreads_ - 1 ? (nextId_ = 0) : nextId_++)]->getEventLoop();
}