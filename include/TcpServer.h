#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <unordered_map>
#include <functional>
#include <memory>

#include "InetAddress.h"
#include "NonCopyable.h"
#include "SpinMutex.h"

#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Buffer.h"
#include "TimerQueue.h"

class TcpServer : NonCopyable {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void (const TcpConnectionPtr &, Buffer &)>;

    TcpServer(const InetAddress &address, const MessageCallback &cb);

    void start(std::size_t numThreads);

private:
    void newConnection(int connectionFd, const InetAddress &peerAddress);

    void closeConnection(const TcpConnectionPtr &connection);

    void closeExpiredConnections();

    using TcpConnectionMap = std::unordered_map<int, TcpConnectionPtr>;
    
    EventLoop loop_;

    std::unique_ptr<EventLoopThreadPool> threadPool_;
    std::unique_ptr<Acceptor> acceptor_;
    InetAddress address_;

    std::unordered_map<TimerId, std::weak_ptr<TcpConnection>> timerConns_;
    TimerQueue timerQueue_;
    Channel timerQueueChannel_;

    SpinMutex mutex_;
    TcpConnectionMap connections_;  // GUARY BY mutex_, 管理所有TcpConnection的生命周期

    MessageCallback messageCallback_;
};
#endif // !__TCP_SERVER_H__