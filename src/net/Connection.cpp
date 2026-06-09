#include "net/Connection.hpp"

#include <cstddef>

#include <sys/socket.h>
#include <sys/types.h>

net::Connection::Connection(int fd) : _socket(fd) {}

int net::Connection::fd() const { return _socket.fd(); }

ssize_t net::Connection::recv(char *buf, std::size_t len) {
    return ::recv(_socket.fd(), buf, len, 0);
}

bool net::Connection::send(const char *data, std::size_t len) {
    std::size_t total = 0;
    while (total < len) {
        ssize_t n =
            ::send(_socket.fd(), data + total, len - total, MSG_NOSIGNAL);
        if (n <= 0) {
            return false;
        }
        total += static_cast<std::size_t>(n);
    }
    return true;
}