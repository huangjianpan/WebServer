#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <functional>

#include "NonCopyable.h"

class EventLoop;

class Channel : NonCopyable {
public:
    using Callback = std::function<void ()>;

    explicit Channel(EventLoop *loop, int fd);

    ~Channel();

    void setReadCallback(Callback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(Callback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(Callback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(Callback cb) { errorCallback_ = std::move(cb); }

    void enableRead() { event_ |= kEnableRead; update(); }
    void enableWrite() { event_ |= kEnableWrite; update(); }
    void disableRead() { event_ &= ~kEnableRead; update(); }
    void disableWrite() { event_ &= ~kEnableWrite; update(); }
    void disableAll() { event_ = 0; update(); }

    bool isNoneEvent() const { return event_ == 0; }
    bool isEnableRead() const { return event_ & kEnableRead; }
    bool isEnableWrite() const { return event_ & kEnableWrite; }

    int status() const { return status_; }
    int fd() const { return fd_; }
    int event() const { return event_; }
    EventLoop *owenrLoop() const { return loop_; }

    void setStatus(int newStatus) { status_ = newStatus; }
    void setRevent(int newRevent) { revent_ = newRevent; }

    void handleEvent();
    void remove();

private:
    void update();

private:
    EventLoop *loop_;
    int fd_;
    int event_;
    int revent_;
    int status_;

    Callback readCallback_;
    Callback writeCallback_;
    Callback closeCallback_;
    Callback errorCallback_;

    static const int kEnableRead;
    static const int kEnableWrite;
};
#endif // !__CHANNEL_H__