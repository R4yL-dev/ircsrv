#include "net/Socket.hpp"

#include <cerrno>
#include <cstring>

#include <fcntl.h>

net::Socket::Socket(int fd) : _handle(fd) {}

int net::Socket::fd() const { return _handle.fd(); }

void net::Socket::setNonBlocking() {
    int flags = fcntl(_handle.fd(), F_GETFL, 0);
    if (flags == -1) {
        throw Socket::Error(std::strerror(errno));
    }
    if (fcntl(_handle.fd(), F_SETFL, flags | O_NONBLOCK) == -1) {
        throw Socket::Error(std::strerror(errno));
    }
}
