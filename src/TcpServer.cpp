#include "TcpServer.h"

TcpServer::TcpServer(const InetAddress &address, const MessageCallback &cb)
    : loop_(), 
    threadPool_(std::make_unique<EventLoopThreadPool>()),
    acceptor_(nullptr),
    address_(address),
    timerQueue(&loop_, std::bind(&TcpServer::closeExpiredConnections, this, std::placeholders::_1)), 
    messageCallback_(cb)
{
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
    timerQueue.addTimer(connection);

    // printf("total TCP connections (new connection) : %lu\n", connections_.size());
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, connection));
}

void TcpServer::closeConnection(const TcpConnectionPtr &connection) {
    {
        std::lock_guard<SpinMutex> lock(mutex_);
        connections_.erase(connection->fd());
    }
    
    // printf("total TCP connections (close connection): %lu\n", connections_.size());
}

void TcpServer::closeExpiredConnections(std::vector<std::weak_ptr<TcpConnection>> &connections) {
    TcpConnectionPtr conn;
    for (auto &p : connections) {
        if ((conn = p.lock()) != nullptr) {
            printf("expired connection fd = %d\n", conn->fd());
            conn->ownerLoop()->runInLoop(std::bind(&TcpServer::closeConnection, this, conn));
        }
    }
}
