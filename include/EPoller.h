#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <vector>
#include <map>
#include <sys/epoll.h>

#include "NonCopyable.h"

class Channel;

class EPoller : NonCopyable {
public:
    using ChannelList = std::vector<Channel *>;

    explicit EPoller();

    ~EPoller();

    void poll(int timeoutOfMs, ChannelList &activeChannels);
    
    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

private:
    void updateEpoll(int operation, Channel *channel);

    using EventList = std::vector<struct epoll_event>;
    using ChannelMap = std::map<int, Channel *>;

    enum { NEW = 0, ADDED = 1, DELETED = 2 };

    const int epollfd_;
    EventList events_;
    ChannelMap channels_;
};
#endif // !__EPOLLER_H__