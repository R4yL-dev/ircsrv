#include "net/tcp.hpp"
#include "net/Socket.hpp"

#include <cerrno>
#include <cstring>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
const int BACKLOG = SOMAXCONN;
} // namespace

// Per-connection failures that are not fatal to the server: the pending
// connection died before we accepted it, or a queued network error surfaced.
// We skip it and try the next one.
static bool isAcceptTransient(int err);
// Resource exhaustion: accepting will keep failing until something frees up.
static bool isAcceptThrottled(int err);
static addrinfo *resolveAddresses(const std::string &host,
                                  const std::string &port, int domain, int type,
                                  int proto);
static int tryBind(const addrinfo *rp);

int net::tcpListen(const std::string &host, int port) {
    std::ostringstream oss;
    oss << port;
    std::string port_str = oss.str();

    struct addrinfo *result =
        resolveAddresses(host, port_str, AF_UNSPEC, SOCK_STREAM, 0);

    int fd = -1;
    for (const addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        fd = tryBind(rp);
        if (fd != -1) {
            break;
        }
    }
    int err = errno;
    freeaddrinfo(result);
    if (fd == -1) {
        throw net::Socket::Error("could not bind to " + host + ": " +
                                 std::strerror(err));
    }

    if (listen(fd, BACKLOG) == -1) {
        int e = errno;
        close(fd);
        throw net::Socket::Error(std::strerror(e));
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        int e = errno;
        close(fd);
        throw net::Socket::Error(std::strerror(e));
    }

    return fd;
}

net::AcceptStatus net::tcpAccept(int listenFd, int &outFd) {
    for (;;) {
        int fd = accept(listenFd, NULL, NULL);
        if (fd != -1) {
            outFd = fd;
            return AcceptOk;
        }
        int err = errno;
        if (err == EINTR) {
            continue;
        }
        if (err == EAGAIN || err == EWOULDBLOCK) {
            return AcceptWouldBlock;
        }
        if (isAcceptThrottled(err)) {
            return AcceptThrottled;
        }
        if (isAcceptTransient(err)) {
            continue; // skip this dead connection, try the next
        }
        throw net::Socket::Error(std::strerror(err));
    }
}

bool isAcceptTransient(int err) {
    return err == ECONNABORTED || err == EPROTO || err == ENETUNREACH ||
           err == EHOSTUNREACH || err == ENONET || err == ENETDOWN ||
           err == EHOSTDOWN;
}

bool isAcceptThrottled(int err) {
    return err == EMFILE || err == ENFILE || err == ENOBUFS || err == ENOMEM;
}

addrinfo *resolveAddresses(const std::string &host, const std::string &port,
                           int domain, int type, int proto) {
    struct addrinfo hints;
    struct addrinfo *result;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    int err = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (err != 0) {
        throw net::Socket::Error(gai_strerror(err));
    }
    return result;
}

int tryBind(const addrinfo *rp) {
    int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
        return -1;
    }

    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        int err = errno;
        close(fd);
        errno = err;
        return -1;
    }

    if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
        return fd;
    }
    int err = errno;
    close(fd);
    errno = err;

    return -1;
}
