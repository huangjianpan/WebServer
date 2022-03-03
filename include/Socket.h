#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "NonCopyable.h"

class InetAddress;

class Socket : NonCopyable {
public:
    explicit Socket(int domain, int type, int protocol);
    
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    int fd() const { return sockfd_; }

    void bind(const InetAddress &addr) const;

    void listen() const;

    bool connect(const InetAddress &addr) const;

    int accept(InetAddress *peerAddr) const;

    int accept(InetAddress &peerAddr) const;

    int accept() const;

    void reusePort() const;

    void reuseAddr() const;

private:
    int sockfd_;
};
#endif // !__SOCKET_H__