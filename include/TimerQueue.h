#ifndef __TIMER_QUEUE_H__
#define __TIMER_QUEUE_H__

#include <sys/timerfd.h>
#include <assert.h>
#include <sys/time.h>

#include <set>
#include <memory>
#include <vector>
#include <functional>

#include "NonCopyable.h"
#include "Socket.h"

class Channel;
class EventLoop;
class TcpConnection;

class TimerQueue : NonCopyable {
private:
    struct Timer {
        long expiredMsec;
        std::weak_ptr<TcpConnection> connection;

        Timer(long expiredMsec_, std::shared_ptr<TcpConnection> connection_) : expiredMsec(expiredMsec_), connection(connection_) {}
    };

    struct TimerLess {
        bool operator()(const Timer &lhs, const Timer &rhs) const {
            return lhs.expiredMsec < rhs.expiredMsec;
        }
    };

public:
    using ExpiredCallback = std::function<void (std::vector<std::weak_ptr<TcpConnection>> &)>;

    explicit TimerQueue(EventLoop *loop, ExpiredCallback cb, long intervalMsec = 10 * 1000);

    ~TimerQueue();

    void addTimer(std::shared_ptr<TcpConnection> conn);

    std::vector<std::weak_ptr<TcpConnection>> &getExpired() {
        return expiredConnections_;
    }

private:
    void handleRead();  // timerfd_

    int timerfd_;
    long intervalMsec_;
    EventLoop *loop_;
    ExpiredCallback cb_;

    std::unique_ptr<Channel> timerfdChannel_;
    std::set<Timer, TimerLess> timers_;
    std::vector<std::weak_ptr<TcpConnection>> expiredConnections_;
};
#endif // !__TIMER_QUEUE_H__