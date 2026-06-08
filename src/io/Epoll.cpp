#include "io/Epoll.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <vector>

#include <sys/epoll.h>

namespace {
const int MAX_EVENTS = 64;
}

static int createEpoll();

io::Epoll::Epoll() : _handle(createEpoll()) {}

void io::Epoll::add(int fd) {
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));

    ev.events = EPOLLIN;
    ev.data.fd = fd;

    int err = epoll_ctl(_handle.fd(), EPOLL_CTL_ADD, fd, &ev);
    if (err < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
}

void io::Epoll::remove(int fd) {
    int err = epoll_ctl(_handle.fd(), EPOLL_CTL_DEL, fd, NULL);
    if (err < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
}

std::vector<io::Event> io::Epoll::wait() {
    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(_handle.fd(), events, MAX_EVENTS, -1);
    if (n < 0) {
        if (errno == EINTR) {
            return std::vector<io::Event>();
        }
        throw io::Epoll::Error(std::strerror(errno));
    }

    std::vector<io::Event> ready;

    for (int i = 0; i < n; i++) {
        io::Event ev;
        ev.fd = events[i].data.fd;
        ev.readable = (events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) != 0;
        ready.push_back(ev);
    }

    return ready;
}

int createEpoll() {
    int fd = epoll_create1(0);
    if (fd < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
    return fd;
}
