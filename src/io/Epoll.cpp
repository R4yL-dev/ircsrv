#include "io/Epoll.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>

#include <stdint.h>
#include <sys/epoll.h>

namespace {
const int MAX_EVENTS = 64;
} // namespace

static uint32_t maskOf(bool read, bool write);
static int createEpoll();

io::Epoll::Epoll() : _handle(createEpoll()) {}

void io::Epoll::add(int fd, bool read, bool write) {
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = maskOf(read, write);
    ev.data.fd = fd;

    if (epoll_ctl(_handle.fd(), EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
    Interest want = {read, write};
    _interest[fd] = want;
}

void io::Epoll::setReadable(int fd, bool on) {
    std::map<int, Interest>::iterator it = _interest.find(fd);
    if (it == _interest.end() || it->second.read == on) {
        return;
    }
    it->second.read = on;
    applyInterest(fd, it->second);
}

void io::Epoll::setWritable(int fd, bool on) {
    std::map<int, Interest>::iterator it = _interest.find(fd);
    if (it == _interest.end() || it->second.write == on) {
        return;
    }
    it->second.write = on;
    applyInterest(fd, it->second);
}

void io::Epoll::remove(int fd) {
    std::map<int, Interest>::iterator it = _interest.find(fd);
    if (it == _interest.end()) {
        return; // not tracked: nothing to do
    }
    _interest.erase(it);

    if (epoll_ctl(_handle.fd(), EPOLL_CTL_DEL, fd, NULL) < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
}

std::vector<io::Event> io::Epoll::wait(int timeoutMs) {
    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(_handle.fd(), events, MAX_EVENTS, timeoutMs);
    if (n < 0) {
        if (errno == EINTR) {
            return std::vector<io::Event>();
        }
        throw io::Epoll::Error(std::strerror(errno));
    }

    std::vector<io::Event> ready;
    for (int i = 0; i < n; i++) {
        uint32_t e = events[i].events;
        io::Event ev;
        ev.fd = events[i].data.fd;
        ev.readable = (e & EPOLLIN) != 0;
        ev.writable = (e & EPOLLOUT) != 0;
        ev.error = (e & EPOLLERR) != 0;
        ev.hup = (e & EPOLLHUP) != 0;
        ready.push_back(ev);
    }

    return ready;
}

void io::Epoll::applyInterest(int fd, const Interest &want) {
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = maskOf(want.read, want.write);
    ev.data.fd = fd;

    if (epoll_ctl(_handle.fd(), EPOLL_CTL_MOD, fd, &ev) < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
}

uint32_t maskOf(bool read, bool write) {
    uint32_t m = 0;
    if (read) {
        m |= EPOLLIN;
    }
    if (write) {
        m |= EPOLLOUT;
    }
    return m;
}

int createEpoll() {
    int fd = epoll_create1(0);
    if (fd < 0) {
        throw io::Epoll::Error(std::strerror(errno));
    }
    return fd;
}
