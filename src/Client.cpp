#include "Client.hpp"
#include <cstddef>
#include <sys/types.h>

namespace {
const std::size_t READ_BUFFER_SIZE = 512;
const std::size_t MAX_MESSAGE_LENGTH = 512;
} // namespace

Client::Client(int fd) : _conn(fd), _inbuf(MAX_MESSAGE_LENGTH) {}

int Client::fd() const { return _conn.fd(); }

bool Client::receive() {
    char buf[READ_BUFFER_SIZE];
    ssize_t n = _conn.recv(buf, sizeof(buf));
    if (n <= 0) {
        return false;
    }
    return _inbuf.append(buf, n);
}

bool Client::getNextMessage(std::string &out) { return _inbuf.getLine(out); }