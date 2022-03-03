#include "InetAddress.h"

#include <arpa/inet.h>
#include <cstring>

InetAddress::InetAddress(in_port_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&addr_.sin_zero, 8);
}

InetAddress::InetAddress(in_port_t port, const char *ip) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr_.sin_addr);
    bzero(&addr_.sin_zero, 8);
}

std::string InetAddress::ip() const {
    char buf[64];
    inet_ntop(family(), &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    return buf;
}

in_port_t InetAddress::port() const { return ntohs(addr_.sin_port); }