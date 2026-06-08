#include "Client.hpp"
#include <cstddef>
#include <sys/types.h>

Client::Client(int fd) : _conn(fd) {}

int Client::fd() const { return _conn.fd(); }

ssize_t Client::recv(char *buf, size_t len) { return _conn.recv(buf, len); }