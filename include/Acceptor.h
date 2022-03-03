#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <functional>

#include "NonCopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"

class EventLoop;

class Acceptor : NonCopyable {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddress &addr)>;

    explicit Acceptor(EventLoop *loop, const InetAddress &addr, NewConnectionCallback callback);

    ~Acceptor();

    void setNewConnectionCallback(NewConnectionCallback callback) { newConnectionCallback_ = std::move(callback); };

private:
    void handleRead();

    int idlefd_;
    EventLoop *loop_;
    Socket socket_;
    Channel channel_;
    InetAddress addr_;
    InetAddress peerAddr_;

    NewConnectionCallback newConnectionCallback_;
};
#endif // !__ACCEPTOR_H__