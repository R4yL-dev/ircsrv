#ifndef NET_TCP_HPP
#define NET_TCP_HPP

#include <string>

namespace net {

// Create a non-blocking listening TCP socket bound to host:port (host may be an
// IPv4 or IPv6 literal, or a name; family is chosen by getaddrinfo). Returns the
// fd, which the caller adopts (e.g. into a Socket). Throws Socket::Error.
int tcpListen(const std::string &host, int port);

enum AcceptStatus {
    AcceptOk,         // a connection was accepted; outFd holds its fd
    AcceptWouldBlock, // no pending connection right now (drain complete)
    AcceptThrottled   // out of resources (EMFILE/ENFILE/ENOBUFS/ENOMEM)
};

// Accept one pending connection on a non-blocking listener. Per-connection
// transient failures (EINTR, ECONNABORTED, already-queued network errors) are
// retried internally to reach the next pending connection. Throws Socket::Error
// on programming errors.
AcceptStatus tcpAccept(int listenFd, int &outFd);

} // namespace net

#endif
