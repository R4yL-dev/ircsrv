#ifndef NET_CONNECTION_HPP
#define NET_CONNECTION_HPP

#include "net/IoStatus.hpp"
#include "net/Socket.hpp"
#include "util/Buffer.hpp"

#include <cstddef>
#include <string>

namespace net {

// A buffered, byte-oriented stream connection: the reusable "byte-pipe".
// Owns the socket plus an inbound and an outbound byte buffer; it knows nothing
// about message framing. Blocking vs non-blocking is the caller's choice via
// setNonBlocking() -- the same API serves both modes (in blocking mode
// WouldBlock never occurs and the outbound buffer drains immediately).
class Connection {
  public:
    Connection(int fd, std::size_t maxSendQueue);

    int fd() const;
    void setNonBlocking();

    // Read side: one recv into the inbound buffer.
    IoStatus fillInput();
    util::Buffer &inbound();

    // Write side: queue bytes, attempting an immediate flush. Returns Ok once
    // the data is safely buffered (even if only partially sent), or Closed if
    // the peer is gone or the send queue would exceed its cap. Never WouldBlock.
    IoStatus queueSend(const char *data, std::size_t len);
    // Flush already-queued output (call on writable readiness): Ok or Closed.
    IoStatus flushOutput();
    bool hasPendingOutput() const;

    // Diagnostic reason set when the connection was deemed Closed (empty
    // otherwise). Copied (not a strerror pointer) so it stays valid.
    const char *closeReason() const;

  private:
    IoStatus flush(); // shared by queueSend and flushOutput

    Socket _socket;
    util::Buffer _in;
    util::Buffer _out;
    std::size_t _maxSendQueue;
    std::string _closeReason;
};

} // namespace net

#endif
