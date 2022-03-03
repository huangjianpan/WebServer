#include <unistd.h>
#include <assert.h>
#include <cstring>

#include "Socket.h"
#include "InetAddress.h"

Socket::Socket(int domain, int type, int protocol)
    : sockfd_(::socket(domain, type, protocol))
{
    if (sockfd_ < 0) {
        printf("%s error in file %s on %d\n", __func__, __FILE__, __LINE__);
    }
}

Socket::~Socket() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

void Socket::bind(const InetAddress &addr) const {
    int ret = ::bind(sockfd_, addr.sockaddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret != 0) {
        printf("%s error in file %s on %d\n", __func__, __FILE__, __LINE__);
    }
}

void Socket::listen() const {
    assert(::listen(sockfd_, 128) == 0);
}

bool Socket::connect(const InetAddress &addr) const {
    return ::connect(sockfd_, addr.sockaddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6))) == 0;
}

int Socket::accept(InetAddress *peerAddr) const {
    if (peerAddr) {
        socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
        return ::accept4(sockfd_, peerAddr->sockaddr(), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    } else {
        return ::accept4(sockfd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    }
}

int Socket::accept(InetAddress &peerAddr) const {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
    return ::accept4(sockfd_, peerAddr.sockaddr(), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

int Socket::accept() const {
    return ::accept4(sockfd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

void Socket::reusePort() const {
    int on = 1;
    assert(::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &on, static_cast<socklen_t>(sizeof on)) == 0);
}

void Socket::reuseAddr() const {
    int on = 1;
    assert(::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &on, static_cast<socklen_t>(sizeof on)) == 0);
}