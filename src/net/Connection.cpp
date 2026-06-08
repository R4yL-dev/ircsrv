#include "net/Connection.hpp"
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>

net::Connection::Connection(int fd) : _socket(fd) {}

int net::Connection::fd() const { return _socket.fd(); }

ssize_t net::Connection::recv(char *buf, size_t len) {
    return ::recv(_socket.fd(), buf, len, 0);
}