#include "net/Connection.hpp"

#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>

namespace {
const std::size_t RECV_CHUNK = 16384;
} // namespace

// errno values that mean "this connection is finished" rather than a bug: the
// peer or the network path is gone. Routine for a server, so we map them to
// Closed instead of throwing.
static bool isConnectionGone(int err);

net::Connection::Connection(int fd, std::size_t maxSendQueue)
    : _socket(fd), _maxSendQueue(maxSendQueue) {}

int net::Connection::fd() const { return _socket.fd(); }

void net::Connection::setNonBlocking() { _socket.setNonBlocking(); }

net::IoStatus net::Connection::fillInput() {
    char tmp[RECV_CHUNK];
    for (;;) {
        ssize_t n = ::recv(_socket.fd(), tmp, sizeof(tmp), 0);
        if (n > 0) {
            _in.append(tmp, static_cast<std::size_t>(n));
            return Ok;
        }
        if (n == 0) {
            _closeReason = "peer closed";
            return Closed;
        }
        int err = errno;
        if (err == EINTR) {
            continue;
        }
        if (err == EAGAIN || err == EWOULDBLOCK) {
            return WouldBlock;
        }
        if (isConnectionGone(err)) {
            _closeReason = std::strerror(err);
            return Closed;
        }
        throw Socket::Error(std::strerror(err));
    }
}

util::Buffer &net::Connection::inbound() { return _in; }

net::IoStatus net::Connection::queueSend(const char *data, std::size_t len) {
    if (_out.size() + len > _maxSendQueue) {
        _closeReason = "send queue exceeded";
        return Closed;
    }
    _out.append(data, len);
    return flush();
}

net::IoStatus net::Connection::flushOutput() { return flush(); }

bool net::Connection::hasPendingOutput() const { return !_out.empty(); }

const char *net::Connection::closeReason() const {
    return _closeReason.c_str();
}

net::IoStatus net::Connection::flush() {
    while (!_out.empty()) {
        ssize_t n = ::send(_socket.fd(), _out.peek(), _out.size(), MSG_NOSIGNAL);
        if (n > 0) {
            _out.consume(static_cast<std::size_t>(n));
            continue;
        }
        if (n == 0) {
            break; // never expected for len > 0; avoid spinning
        }
        int err = errno;
        if (err == EINTR) {
            continue;
        }
        if (err == EAGAIN || err == EWOULDBLOCK) {
            break; // socket full: leave the rest buffered for EPOLLOUT
        }
        if (isConnectionGone(err)) {
            _closeReason = std::strerror(err);
            return Closed;
        }
        throw Socket::Error(std::strerror(err));
    }
    return Ok;
}

bool isConnectionGone(int err) {
    return err == ECONNRESET || err == EPIPE || err == ETIMEDOUT ||
           err == EHOSTUNREACH || err == ENETUNREACH || err == ENETDOWN ||
           err == ECONNABORTED;
}
