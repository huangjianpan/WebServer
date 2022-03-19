#include "TcpServer.h"

TcpServer::TcpServer(const InetAddress &address, const MessageCallback &cb)
    : loop_(), 
    threadPool_(std::make_unique<EventLoopThreadPool>()),
    acceptor_(nullptr),
    address_(address),
    timerQueue_(2 * 1000), 
    timerQueueChannel_(&loop_, timerQueue_.timerfd()),
    messageCallback_(cb)
{
    timerQueueChannel_.setReadCallback(std::bind(&TcpServer::closeExpiredConnections, this));
    timerQueueChannel_.enableRead();
    threadPool_->setBaseLoop(&loop_);
    acceptor_.reset(new Acceptor(&loop_, address_, 
                        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)));
}

void TcpServer::start(std::size_t numThreads) {
    threadPool_->start(numThreads);
    loop_.loop();
}

void TcpServer::newConnection(int connectionFd, const InetAddress &peerAddress) {
    EventLoop *ioLoop(threadPool_->getNextLoop());
    TcpConnectionPtr connection(std::make_shared<TcpConnection>(ioLoop, connectionFd, peerAddress));
    connection->setMessageCallback(messageCallback_);
    connection->setCloseCallback(std::bind(&TcpServer::closeConnection, this, std::placeholders::_1));

    {
        std::lock_guard<SpinMutex> lock(mutex_);
        connections_.emplace(connectionFd, connection);
    }
    TimerId id = timerQueue_.addTimer(20 * 1000);
    timerConns_.emplace(id, connection);

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, connection));
}

void TcpServer::closeConnection(const TcpConnectionPtr &connection) {
    {
        std::lock_guard<SpinMutex> lock(mutex_);
        connections_.erase(connection->fd());
    }
}

void TcpServer::closeExpiredConnections() {
    timerQueue_.handleExpired();
    for (Timer& timer : timerQueue_.getExpiredTimers()) {
        auto it = timerConns_.find(timer.id);
        if (it != timerConns_.end()) {
            auto conn = it->second.lock();
            if (conn) {
                printf("expired connection fd = %d, timerId = %lu, expiredTime = %ld\n", conn->fd(), timer.id, timer.expiredMsec);
                conn->ownerLoop()->runInLoop(std::bind(&TcpServer::closeConnection, this, conn));
            }
            timerConns_.erase(it);
        }
    }
}
