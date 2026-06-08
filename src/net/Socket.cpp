#include "net/Socket.hpp"

#include <string>

#include <unistd.h>

net::Socket::Socket(int fd) {
    if (fd < 0) {
        throw net::Socket::Error("invalid fd");
    }
    _fd = fd;
}

net::Socket::~Socket() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int net::Socket::fd() const { return _fd; }
