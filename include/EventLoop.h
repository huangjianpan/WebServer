#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <memory>
#include <vector>
#include <mutex>
#include <functional>

#include "NonCopyable.h"
#include "ThisThread.h"

class EPoller;
class Channel;

class EventLoop : NonCopyable {
public:
    using Functor = std::function<void ()>;

    EventLoop();

    ~EventLoop();

    void loop();

    void quit() { quit_ = true; wakeup(); };

    void runInLoop(Functor functor);

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool isInLoopThread() const { return tid_ == ThisThread::tid(); }

    void wakeup() const;

private:
    void handleWakeup() const;

    void handlePendingFunctors();

    using ChannelList = std::vector<Channel *>;
    using FunctorList = std::vector<Functor>;

    bool looping_;
    bool quit_;
    pid_t tid_;

    std::unique_ptr<EPoller> poller_;
    ChannelList activeChannels_;

    std::mutex mtx_;
    FunctorList pendingFunctors_;
    std::unique_ptr<Channel> wakeupChannel_;
};
#endif // !__EVENT_LOOP_H__