#include <assert.h>
#include <sys/eventfd.h>

#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"

EventLoop::EventLoop() : 
    looping_(false),
    quit_(false),
    tid_(ThisThread::tid()),
    poller_(std::make_unique<EPoller>()),
    wakeupChannel_(std::make_unique<Channel>(this, eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)))
{
    assert(poller_);
    assert(wakeupChannel_->fd() >= 0);
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
    wakeupChannel_->enableRead();
}

EventLoop::~EventLoop() {
    wakeupChannel_->remove();
    close(wakeupChannel_->fd());
}

void EventLoop::loop() {
    assert(!quit_);
    assert(!looping_);
    assert(isInLoopThread());

    looping_ = true;
    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(-1, activeChannels_);
        for (auto channel : activeChannels_) {
            channel->handleEvent();
        }
        handlePendingFunctors();
    }
    looping_ = false;
}

void EventLoop::runInLoop(Functor functor) {
    if (isInLoopThread()) {
        functor();
    } else {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            pendingFunctors_.emplace_back(std::move(functor));            
        }
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    if (channel->owenrLoop() == this) {
        poller_->removeChannel(channel);
    }
}

void EventLoop::wakeup() const {
    uint64_t v = 1;
    ssize_t n = write(wakeupChannel_->fd(), &v, sizeof v);
    if (n < 0) {
        printf("%s, error = %d\n", __func__, errno);
    }
}

void EventLoop::handleWakeup() const {
    uint64_t v = 0;
    ssize_t n = read(wakeupChannel_->fd(), &v, sizeof v);
    if (n < 0) {
        printf("%s, error = %d\n", __func__, errno);
    }
}

void EventLoop::handlePendingFunctors() {
    decltype(pendingFunctors_) functors;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        std::swap(functors, pendingFunctors_);
    }
    for (const auto &f : functors) {
        f();
    }
}