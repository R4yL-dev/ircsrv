#include "io/SignalFd.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>

#include <sys/signalfd.h>
#include <sys/types.h>
#include <unistd.h>

static int createSignalFd(const sigset_t &mask);

io::SignalFd::SignalFd(const sigset_t &mask) : _handle(createSignalFd(mask)) {}

int io::SignalFd::fd() const { return _handle.fd(); }

int io::SignalFd::readSignal() {
    struct signalfd_siginfo si;
    ssize_t n = ::read(_handle.fd(), &si, sizeof(si));

    if (n != static_cast<ssize_t>(sizeof(si))) {
        throw io::SignalFd::Error("incomplete signalfd read");
    }

    return static_cast<int>(si.ssi_signo);
}

int createSignalFd(const sigset_t &mask) {
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        throw io::SignalFd::Error(std::strerror(errno));
    }

    int fd = signalfd(-1, &mask, 0);
    if (fd == -1) {
        throw io::SignalFd::Error(std::strerror(errno));
    }
    return fd;
}