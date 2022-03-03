#include <assert.h>
#include <unistd.h>

#include "EPoller.h"
#include "Channel.h"

EPoller::EPoller() : 
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(16)
{
    assert(epollfd_ >= 0);
}

EPoller::~EPoller() {
    close(epollfd_);
}

void EPoller::poll(int timeoutOfMs, ChannelList &activeChannels) {
    int num = epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutOfMs);
    if (num > 0) {
        activeChannels.reserve(num);
        for (int i = 0; i < num; ++i) {
            Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
            channel->setRevent(events_[i].events);
            activeChannels.emplace_back(channel);
        }
        if (num == static_cast<int>(events_.size())) {
            events_.resize(2 * num);
        }
    } else if (num == 0) {
        printf("nothing happened\n");
    } else {
        printf("%s error in file %s on %d, errno = %d\n", __func__, __FILE__, __LINE__, errno);
    }
}

void EPoller::updateChannel(Channel *channel) {
    int status = channel->status();
    int fd = channel->fd();
    if (status == NEW) {
        channels_.emplace(fd, channel);
        updateEpoll(EPOLL_CTL_ADD, channel);
        channel->setStatus(ADDED);
    } else if (status == DELETED) {
        updateEpoll(EPOLL_CTL_ADD, channel);
        channel->setStatus(ADDED);
    } else {
        if (channel->isNoneEvent()) {
            updateEpoll(EPOLL_CTL_DEL, channel);
            channel->setStatus(DELETED);
        } else {
            updateEpoll(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel *channel) {
    assert(channel);
    
    int status = channel->status();
    if (status != NEW) {
        channels_.erase(channel->fd());
    }
    if (status == ADDED) {
        updateEpoll(EPOLL_CTL_DEL, channel);
    }
    channel->setStatus(NEW);
}

void EPoller::updateEpoll(int operation, Channel *channel) {
    struct epoll_event ev;
    ev.events = channel->event();
    ev.data.ptr = static_cast<void *>(channel);
    if (epoll_ctl(epollfd_, operation, channel->fd(), &ev) < 0) {
        printf("%s, error = %d\n", __func__, errno);
    }
}