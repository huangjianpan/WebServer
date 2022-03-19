#include "TimerQueue.h"

TimerQueue::TimerQueue(long intervalMsec) 
    : intervalMsec_(intervalMsec), timerId_(0)
{
    if ((timerfd_ = ::timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC)) < 0) {
        printf("%s error in file %s on line %d, errno = %d\n", __func__, __FILE__, __LINE__, errno);
        exit(1);
    }
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

void TimerQueue::handleExpired() {
    {
        uint64_t val = 0;
        ssize_t nread = ::read(timerfd_, &val, sizeof(uint64_t));
    }

    assert(timers_.size() == expiredMsec_.size());

    if (timers_.empty()) return;

    long currentMsec = 0;
    {
        struct timeval time;
        ::gettimeofday(&time, nullptr);
        currentMsec = time.tv_sec * 1000 + time.tv_usec / 1000;
    }

    expiredTimers_.clear();

    auto last = timers_.begin();
    while (last != timers_.end() && currentMsec >= last->expiredMsec) {
        expiredMsec_.erase(last->id);
        expiredTimers_.emplace_back(last->id, last->expiredMsec);
        ++last;
    }
    timers_.erase(timers_.begin(), last);
}

TimerId TimerQueue::addTimer(long expiredMsecFromNow) {
    assert(expiredMsecFromNow > 0);
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    long expiredTime = tv.tv_sec * 1000 + tv.tv_usec / 1000 + expiredMsecFromNow;
    timers_.emplace(++timerId_, expiredTime);
    expiredMsec_.emplace(timerId_, expiredTime);
    return timerId_;
}

void TimerQueue::deleteTimer(TimerId timerId) {
    auto it = expiredMsec_.find(timerId);
    if (it == expiredMsec_.end()) {
        return;
    }

    expiredMsec_.erase(it);
    timers_.erase(Timer(timerId, it->second));
}

void TimerQueue::updateTimer(TimerId timerId, long expiredMsecFromNow) {
    assert(expiredMsecFromNow > 0);
    auto it = expiredMsec_.find(timerId);
    if (it == expiredMsec_.end()) {
        return;
    }
    
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);

    timers_.erase(Timer(timerId, it->second));
    it->second = tv.tv_sec * 1000 + tv.tv_usec / 1000 + expiredMsecFromNow;
    timers_.emplace(timerId, it->second);
}
