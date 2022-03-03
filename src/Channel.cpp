#include <sys/epoll.h>
#include <assert.h>

#include "EventLoop.h"
#include "Channel.h"

const int Channel::kEnableRead = EPOLLIN | EPOLLPRI;
const int Channel::kEnableWrite = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) :
    loop_(loop),
    fd_(fd),
    event_(0),
    revent_(0), 
    status_(0)
{
    assert(loop_);
}

Channel::~Channel() {
    loop_->removeChannel(this);
}

void Channel::handleEvent() {
    if (revent_ & EPOLLHUP) {
        if (closeCallback_) closeCallback_();
    }
    if (revent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) readCallback_();
    }
    if (revent_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    if (revent_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::update() {
    loop_->updateChannel(this);
}