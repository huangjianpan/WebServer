#include <unistd.h>

#include "TcpConnection.h"
#include "EventLoop.h"

TcpConnection::TcpConnection(EventLoop *loop, int fd, const InetAddress &peerAddress) :
    loop_(loop),
    socket_(fd),
    channel_(loop, fd),
    state_(kConnecting),
    peerAddr_(peerAddress)
{
    channel_.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_.setErrorCallback(std::bind(&TcpConnection::handleError, this));
    assert(loop_);
    printf("establish TCP connection: \n     ip:port = %s:%d, fd = %d\n", peerAddr_.ip().c_str(), peerAddr_.port(), socket_.fd());
}

TcpConnection::~TcpConnection() {
    printf("close TCP connection: \n     ip:port = %s:%d, fd = %d\n", peerAddr_.ip().c_str(), peerAddr_.port(), socket_.fd());
}

void TcpConnection::connectEstablished() {
    assert(state_ == kConnecting);
    channel_.enableRead();
    setState(kConnected);
}

void TcpConnection::send(const char *message, size_t length) {
    if (state_ == kConnected) {
        loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message, length));
    }
}

void TcpConnection::sendInLoop(const void *message, std::size_t length) {
    if (state_ != kConnected) return;
    ssize_t nwrite = 0, remaining = length;
    if (!channel_.isEnableWrite()) {
        nwrite = write(socket_.fd(), message, length);
        if (nwrite >= 0) {
            remaining = length - nwrite;
        } else {
            handleError();
            return;
        }
    }
    if (remaining > 0) {
        outputBuffer_.append(static_cast<const char *>(message) + nwrite, remaining);
        if (!channel_.isEnableWrite()) {
            channel_.enableWrite();
        }
    }
}

void TcpConnection::handleRead() {
    ssize_t n = inputBuffer_.read(socket_.fd());
    if (n > 0) {
        if (messageCallback_) messageCallback_(shared_from_this(), inputBuffer_);
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    ssize_t n = write(socket_.fd(), outputBuffer_.first(), outputBuffer_.readableBytes());
    if (n >= 0) {
        outputBuffer_.retrieve(n);
        if (outputBuffer_.readableBytes() == 0) {
            channel_.disableWrite();
        }
    } else {
        outputBuffer_.retrieveAll();
        channel_.disableWrite();
        handleError();
    }
}

void TcpConnection::handleError() {
    printf("connection error, fd = %d\n", fd());
    handleClose();
}

void TcpConnection::handleClose() {
    channel_.remove();
    setState(kDisconnected);
    closeCallback_(shared_from_this());
}