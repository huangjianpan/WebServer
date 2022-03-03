#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const InetAddress &addr, NewConnectionCallback callback) :
    idlefd_(open("/dev/null", O_CLOEXEC)),
    loop_(loop),
    socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP),
    channel_(loop, socket_.fd()),
    addr_(addr),
    newConnectionCallback_(callback)
{
    assert(idlefd_ >= 0);
    assert(loop_);
    assert(socket_.fd());

    socket_.reuseAddr();
    socket_.reusePort();
    socket_.bind(addr_);

    channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
    channel_.enableRead();
    socket_.listen();
}

Acceptor::~Acceptor() {
    close(idlefd_);
    channel_.disableAll();
}

void Acceptor::handleRead() {
    int cfd = socket_.accept(peerAddr_);
    // printf("%s:%d, fd = %d\n", peerAddr_.ip().c_str(), peerAddr_.port(), cfd);
    if (cfd >= 0) {
        newConnectionCallback_(cfd, peerAddr_);
    } else {
        printf("%s error in file %s on %d, errno = %d\n", __func__, __FILE__, __LINE__, errno);
        if (errno == EMFILE) {
            close(idlefd_);
            idlefd_ = socket_.accept();
            close(idlefd_);
            idlefd_ = open("/dev/null", O_CLOEXEC);
        }
    }
}