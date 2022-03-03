#include "TimerQueue.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpConnection.h"

TimerQueue::TimerQueue(EventLoop *loop, ExpiredCallback cb, long intervalMsec) 
    : intervalMsec_(intervalMsec), loop_(loop), cb_(cb)
{
    assert(loop_);
    if ((timerfd_ = ::timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC)) < 0) {
        printf("%s error in file %s on line %d, errno = %d\n", __func__, __FILE__, __LINE__, errno);
        exit(-1);
    }
    timerfdChannel_ = std::make_unique<Channel>(loop_, timerfd_);
    timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_->enableRead();
    struct timespec tv;
    tv.tv_sec = intervalMsec / 1000;
    tv.tv_nsec = (intervalMsec % 1000) * 1000000;
    struct itimerspec time;
    time.it_interval = time.it_value = tv;
    ::timerfd_settime(timerfd_, 0, &time, &time);
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
}

void TimerQueue::handleRead() {
    {
        uint64_t val = 0;
        ssize_t nread = ::read(timerfd_, &val, sizeof(uint64_t));
    }
    if (timers_.empty()) return;
    struct timeval time;
    ::gettimeofday(&time, nullptr);
    long currentMsec = time.tv_sec * 1000 + time.tv_usec / 1000;
    auto last = timers_.begin();
    while (last != timers_.end() && currentMsec >= last->expiredMsec) {
        ++last;
    }
    expiredConnections_.clear();
    for (auto first = timers_.begin(); first != last; ++first) {
        if (!first->connection.expired()) {
            expiredConnections_.emplace_back(first->connection);
        }
    }
    timers_.erase(timers_.begin(), last);
    if (cb_ && !expiredConnections_.empty()) cb_(expiredConnections_);
}

void TimerQueue::addTimer(std::shared_ptr<TcpConnection> conn) {
    struct timeval time;
    ::gettimeofday(&time, nullptr);
    timers_.emplace(time.tv_sec * 1000 + time.tv_usec / 1000 + intervalMsec_, conn);
}