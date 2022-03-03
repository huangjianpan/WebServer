#ifndef __INET_ADDRESS_H__
#define __INET_ADDRESS_H__

#include <netinet/in.h>
#include <string>

class InetAddress {
public:
    explicit InetAddress(in_port_t port = 0);
    explicit InetAddress(in_port_t port, const char *ip);
    explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

    InetAddress(const InetAddress &rhs) : addr_(rhs.addr_) {}

    InetAddress &operator=(const InetAddress &rhs) {
        addr_ = rhs.addr_;
        return *this;
    }

    sa_family_t family() const { return addr_.sin_family; }

    std::string ip() const;

    in_port_t port() const;

    struct sockaddr *sockaddr() {
        return (struct sockaddr *) &addr_;
    }

    const struct sockaddr *sockaddr() const {
        return (const struct sockaddr *) &addr_;
    }

private:
    struct sockaddr_in addr_;
};
#endif // !__INET_ADDRESS_H__