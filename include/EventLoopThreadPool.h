#ifndef __EVENT_LOOP_THREAD_POOL_H__
#define __EVENT_LOOP_THREAD_POOL_H__

#include <vector>
#include <memory>

#include "NonCopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : NonCopyable {
public:
    EventLoopThreadPool();

    void setBaseLoop(EventLoop *baseLoop) { baseLoop_ = baseLoop; }

    void start(std::size_t numThreads);

    EventLoop *getNextLoop();

private:
    bool start_;
    EventLoop *baseLoop_;
    std::size_t nextId_;
    std::size_t numThreads_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
};
#endif // !__EVENT_LOOP_THREAD_POOL_H__