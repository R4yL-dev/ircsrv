#include "io/FdHandle.hpp"

#include <unistd.h>

io::FdHandle::FdHandle(int fd) : _fd(fd) {}

io::FdHandle::~FdHandle() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int io::FdHandle::fd() const { return _fd; }

int io::FdHandle::release() {
    int fd = _fd;

    _fd = -1;

    return fd;
}