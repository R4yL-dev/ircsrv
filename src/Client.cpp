#include "Client.hpp"

#include <cstddef>

namespace {
const std::size_t MAX_MESSAGE_LENGTH = 510; // RFC 2812 line length cap
} // namespace

// The ctor adopts the fd but does no throwing work, so acceptClients' RAII guard
// stays leak-safe; setNonBlocking() is called separately once the Client owns it.
Client::Client(int fd, std::size_t maxSendQueue)
    : _conn(fd, maxSendQueue), _framer(MAX_MESSAGE_LENGTH) {}

int Client::fd() const { return _conn.fd(); }

void Client::setNonBlocking() { _conn.setNonBlocking(); }

net::IoStatus Client::receive() { return _conn.fillInput(); }

bool Client::getNextMessage(std::string &out) {
    return _framer.nextLine(_conn.inbound(), out);
}

net::IoStatus Client::queueSend(const char *data, std::size_t len) {
    return _conn.queueSend(data, len);
}

net::IoStatus Client::flush() { return _conn.flushOutput(); }

bool Client::hasPendingOutput() const { return _conn.hasPendingOutput(); }

const char *Client::closeReason() const { return _conn.closeReason(); }
