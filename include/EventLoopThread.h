#ifndef __EVENT_LOOP_THREAD_H__
#define __EVENT_LOOP_THREAD_H__

#include <thread>
#include <mutex>
#include <condition_variable>

#include "NonCopyable.h"

class EventLoop;

class EventLoopThread : NonCopyable {
public:
    explicit EventLoopThread();
    ~EventLoopThread() { terminate(); }

    void terminate();

    EventLoop *getEventLoop() { return loop_; };

private:
    void threadFunc();

    EventLoop *loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
#endif // !__EVENT_LOOP_THREAD_H__