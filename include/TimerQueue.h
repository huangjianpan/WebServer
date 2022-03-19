#ifndef __TIMER_QUEUE_H__
#define __TIMER_QUEUE_H__

#include <sys/timerfd.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include <vector>
#include <set>
#include <unordered_map>

#include "NonCopyable.h"

class TimerQueue;
class TimerCompare;

using TimerId = std::size_t;

class Timer {
public:
    explicit Timer(TimerId id_, long expiredMsec_) 
        : id(id_), expiredMsec(expiredMsec_) {}

    bool operator==(const Timer& rhs) const {
        return id == rhs.id;
    }

    const TimerId id;
    const long expiredMsec;
};

struct TimerCompare {
    bool operator()(const Timer& lhs, const Timer& rhs) const {
        return (lhs.expiredMsec == rhs.expiredMsec) ? 
                    (lhs.id < rhs.id) : 
                    (lhs.expiredMsec < rhs.expiredMsec);
    }
};

class TimerQueue : NonCopyable {
public:
    TimerQueue(long intervalMsec);

    ~TimerQueue();

    void handleExpired();

    int timerfd() const { return timerfd_; }

    std::vector<Timer> &getExpiredTimers() { return expiredTimers_; }

    TimerId addTimer(long expiredMsecFromNow);

    void deleteTimer(TimerId timerId);

    void updateTimer(TimerId timerId, long expiredMsecFromNow);

private:
    int timerfd_;
    long intervalMsec_;
    TimerId timerId_;

    std::set<Timer, TimerCompare> timers_;
    std::unordered_map<TimerId, long> expiredMsec_;

    std::vector<Timer> expiredTimers_;
};
#endif // !__TIMER_QUEUE_H__