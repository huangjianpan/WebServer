#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include <functional>
#include <memory>

#include "NonCopyable.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Socket.h"
#include "Buffer.h"

class EventLoop;

class TcpConnection : NonCopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void (const TcpConnectionPtr &, Buffer &)>;
    using CloseCallback = std::function<void (const TcpConnectionPtr &)>;

    explicit TcpConnection(EventLoop *loop, int fd, const InetAddress &peerAddress);

    ~TcpConnection();

    void setMessageCallback(MessageCallback callback) { messageCallback_ = std::move(callback); }

    void setCloseCallback(CloseCallback callback) { closeCallback_ = std::move(callback); };

    void send(const char *message, size_t length);

    void close() { handleClose(); };

    bool isConnected() const { return state_ == kConnected; }

    int fd() const { return socket_.fd(); }

    const InetAddress &peerAddress() const { return peerAddr_; }

    EventLoop *ownerLoop() const { return loop_; }

    void connectEstablished();

private:
    void setState(int newState) { state_ = newState; };

    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();

    void sendInLoop(const void *message, std::size_t length);

    enum { kConnecting, kConnected, kDisconnecting, kDisconnected };

    EventLoop *loop_;
    Socket socket_;
    Channel channel_;
    int state_;
    const InetAddress peerAddr_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};
#endif // !__TCP_CONNECTION_H__