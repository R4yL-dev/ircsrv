#include "net/Socket.hpp"

net::Socket::Socket(int fd) : _handle(fd) {}

int net::Socket::fd() const { return _handle.fd(); }
